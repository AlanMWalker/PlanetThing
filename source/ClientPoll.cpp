#include "ClientPoll.h"
#include <KApplication.h>
#include <ctime>
#include "NetworkUtils.h"

using namespace std;
using namespace Krawler;


ClientPoll::ClientPoll()
{
}

ClientPoll::~ClientPoll()
{
	
}

void ClientPoll::spawnIn(const Vec2f& pos)
{
	if (m_bDidSpawnInWorld)
	{
		return;
	}

	lock_guard<mutex> lock(m_queueMutex);
	sf::Packet p;
	MoveInWorld siw;
	siw.playerPosition = pos;
	siw.playerName = m_networkInfo.playerName;
	p << siw;
	m_messageQueue.push_back(p);
	m_bDidSpawnInWorld = true;
}

void ClientPoll::moveInWorld(const Vec2f& pos)
{
	lock_guard<mutex> lock(m_queueMutex);
	sf::Packet p;
	MoveInWorld move;
	move.playerPosition = pos;
	move.playerName = m_networkInfo.playerName;
	move.timeStamp = timestamp();
	p << move;
	m_messageQueue.push_back(p);
	m_bDidSpawnInWorld = true;
}

void ClientPoll::subscribeToPacketType(MessageType type, std::function<void(ServerClientMessage*)>* funcRef)
{
	m_typeSubscribers[type].push_back(funcRef);
}

void ClientPoll::closeComms()
{
	m_bCommsAlive = false;
	m_networkPollThread.join();
	m_connSocket.unbind();
}

void ClientPoll::loadClient()
{
	loadNetworkConf();
	KPrintf(L"Network info: \nServer IP : %s \nServer Port : %d \nMy Port : %d\n",
		TO_WSTR(m_networkInfo.serverIp.toString()).c_str(), m_networkInfo.serverPort, m_networkInfo.myPort);

	m_connSocket.setBlocking(false);
	m_networkPollThread = thread(&ClientPoll::connectToServer, this);
	m_bCommsAlive = true;
}

void ClientPoll::loadNetworkConf()
{
	wifstream networkFile(L"network.cfg", ios::in);

	if (networkFile.fail())
	{
		KPRINTF("Unable to load network configuration! Exiting game!\n");
		GET_APP()->closeApplication();
		return;
	}
	wstring line;
	wstring serverIp = L"";
	unsigned short serverPort = 0;
	unsigned short myPort = 0;
	while (!networkFile.eof())
	{
		line.clear();
		line.resize(100);
		networkFile.getline(&line[0], 100, '\n');
		if (line[0] == '#')
		{
			continue;
		}
		else
		{
			break;
		}
	}

	serverIp = line;
	networkFile >> serverPort;
	networkFile >> myPort;

	if (line.size() == 0 || serverPort == 0 || myPort == 0)
	{
		KPRINTF("Unable to load network configuration! Exiting game!\n");
		GET_APP()->closeApplication();
	}

	memset(&m_networkInfo, 0, sizeof(NetworkInfo));
	m_networkInfo.myPort = myPort;
	m_networkInfo.serverPort = serverPort;
	m_networkInfo.serverIp = TO_ASTR(serverIp);
}


void ClientPoll::connectToServer()
{
	// First we try bind to the UDP socket
	// that we're going to send/receive data on from 
	// the server, if we can't we'll exit for now 
	// (should display onscreen error in future)
	auto bindResult = sf::Socket::Error;
	while (m_networkInfo.myPort < 32004 && bindResult != sf::Socket::Done)
	{
		bindResult = m_connSocket.bind(m_networkInfo.myPort);
		if (bindResult != sf::Socket::Done)
		{
			m_networkInfo.myPort += 1;
		}
	}

	if (bindResult != sf::Socket::Done)
	{
		KPrintf(L"Unable to bind to socket! Exiting game\n");
		GET_APP()->closeApplication();
	}

	EstablishConnection handshake;
	handshake.clientVersion = TO_ASTR(GAME_CLIENT_VERSION);

	sf::Packet p;
	auto sendHandShake = [this, &p, &handshake]()
	{
		p.clear();
		p << handshake;
		KPRINTF("Initiating handshake with GameMaster\n");
		m_connSocket.send(p, m_networkInfo.serverIp, m_networkInfo.serverPort);
	};

	sendHandShake();

	p.clear();

	bool bDidFail = false;
	sf::Clock elapsed;

	while (!m_bConnEstablished && !bDidFail && m_bCommsAlive)
	{

		sf::IpAddress incomingIp;
		unsigned short incomingPort;
		auto socketResult = m_connSocket.receive(p, incomingIp, incomingPort);
		const int asSeconds = elapsed.getElapsedTime().asSeconds();
		if ((asSeconds > 1) && (asSeconds % 5) == 0)
		{
			sendHandShake();
			elapsed.restart();
		}

		if (asSeconds > MAX_RETRY_TIME)
		{
			KPrintf(L"Error - No response from server after %d amount of time\n");
			bDidFail = true;
			continue;
		}

		if (socketResult == sf::Socket::NotReady)
		{
			continue;
		}

		if (incomingIp == m_networkInfo.serverIp && !bDidFail)
		{
			int connEstablishReply = 0;
			p >> connEstablishReply;

			if (!connEstablishReply)
			{
				KPrintf(L"Error - Server killed connection!\n");
				/*GET_APP()->closeApplication();*/
				bDidFail = true;
			}
			else
			{
				m_bConnEstablished = true;
				KPrintf(L"Established connection to GameMaster!\n");
			}

		}
	}

	if (bDidFail)
	{
		KPRINTF("Error - Shutting down game after connection failure!\n");
	}
	else
	{
		networkPollLoop();
	}
}

void ClientPoll::networkPollLoop()
{
	KPRINTF("Network Poll Loop Initiated\n");
	constexpr int32 MAX_TIME = 16;
	while (m_bCommsAlive)
	{
		bool bCouldLock = false;
		const auto t1 = chrono::high_resolution_clock::now();
		lock_guard<mutex> lock(m_socketMutex);

		if (m_bConnEstablished)
		{
			processQueue();
		}

		sf::IpAddress sender;
		unsigned short port = 0;
		sf::Packet p;
		const auto socketResult = m_connSocket.receive(p, sender, port);
		if (socketResult == sf::Socket::Error)
		{
			KPrintf(L"Server down! Exiting \n");
			GET_APP()->closeApplication();
		}

		if (socketResult == sf::Socket::Done)
		{
			if (sender == m_networkInfo.serverIp)
			{
				if (m_bConnEstablished)
				{
					handleReplies(p);
				}
			}
		}

		const auto t2 = chrono::high_resolution_clock::now();
		const auto sleepTime = chrono::milliseconds(MAX_TIME) - chrono::duration_cast<chrono::milliseconds>((t2 - t1));
		this_thread::sleep_for(sleepTime);
	}
}

void ClientPoll::handleReplies(sf::Packet& p)
{
	auto v = (ServerClientMessage*)p.getData();

	const char* const pScm = (char*)p.getData();
	const int32 typeAsInt = (pScm[0] << 24) | (pScm[1] << 16) | (pScm[2] << 8) | (pScm[3]);

	switch (static_cast<MessageType>(typeAsInt))
	{
	case MessageType::Move:
	{
		MoveInWorld siw;
		p >> siw;
		for (auto callback : m_typeSubscribers[siw.type])
		{
			(*callback)(&siw);
		}
		break;
	}
	case MessageType::Disconnect:
		KPRINTF("Server kicked client! Closing game\n");
		GET_APP()->closeApplication();
		break;
	case MessageType::KeepAlive:
		KeepAlive ka;
		p >> ka;

		if (ka.message == "DONE")
		{
			m_serverResponseTime = m_lastSentTimestamp - timestamp();
		}
		break;
	}
}

void ClientPoll::processQueue()
{
	constexpr int32 PROCESS_QUEUE_TIME = 16000;
	static sf::Clock c; 
	
	sf::Packet p;
	KeepAlive ka;

	if (c.getElapsedTime().asSeconds() >= 2)
	{
		ka.message = "ALIVE";
		ka.timeStamp = timestamp();
		m_lastSentTimestamp = ka.timeStamp;
		p << ka;
		c.restart();
		m_connSocket.send(p, m_networkInfo.serverIp, m_networkInfo.serverPort);
	}

	p.clear();

	lock_guard<mutex> lock(m_queueMutex);

	const chrono::high_resolution_clock::time_point timeA = chrono::high_resolution_clock::now();
	chrono::high_resolution_clock::time_point timeB = chrono::high_resolution_clock::now();

	// if we take more than 5 ms to process the queue we'll wait till next cycle to use it
	while (chrono::duration_cast<chrono::microseconds>(timeB - timeA) < chrono::microseconds(16000) &&
		m_messageQueue.size() > 0)
	{
		auto front = m_messageQueue.front();
		m_connSocket.send(front, m_networkInfo.serverIp, m_networkInfo.serverPort);
		m_messageQueue.pop_front();
		timeB = chrono::high_resolution_clock::now();
	}
}

