#include "ServerPoll.h"
#include "NetworkUtils.h"

using namespace std;
using namespace std::chrono;

ServerPoll::ServerPoll()
{
}

bool ServerPoll::loadServer()
{
	if (m_commsSocket.bind(SERVER_LISTENING_PORT) != sf::Socket::Done)
	{
		cout << "Unable to start server on port " << SERVER_LISTENING_PORT << endl;
		return false;
	}

	m_commsSocket.setBlocking(false);
	return true;
}

void ServerPoll::runServer()
{
	sf::Clock serverClock;
	while (true)
	{
		const sf::Time tBefore = serverClock.getElapsedTime();
		const int result = handleReceived();
		if (result != EXIT_SUCCESS)
		{
			cout << "Server failed to receive messages correctly, shutting down" << endl;
			break;
		}

		pollConnected();
		processQueue();

		const sf::Time tAfter = serverClock.getElapsedTime();
		const int TimeTaken = (tAfter - tBefore).asMicroseconds();
		const int SleepTime = SERVER_DELTA_TIME - TimeTaken;

		if (m_bLogExeTime)
		{
			if (m_frames > 10)
			{
				Krawler::KPrintf(L"Execution time is %d microsec \n", TimeTaken);
				m_frames = 0;
			}
			else
			{
				++m_frames;
			}
		}
		//this_thread::sleep_for(milliseconds(SleepTime));
	}
}

void ServerPoll::pollConnected()
{
	vector<GameClient> kicked;
	for (auto& c : m_connectedClients)
	{
		auto delta = chrono::system_clock::now() - c.lastPollTime;
		chrono::seconds deltaAsSeconds = chrono::duration_cast<chrono::seconds>(delta);
		if (deltaAsSeconds > chrono::seconds(CLIENT_TIMEOUT_IN_SECONDS))
		{
			cout << "Disconnecting client " << c.ip << ":" << c.port << std::endl;
			kicked.push_back(c);
		}
	}

	for (auto& k : kicked)
	{
		auto r = std::find_if(m_connectedClients.begin(), m_connectedClients.end(), [&k](const GameClient& c) -> bool
			{
				return c == k;
			});

		if (r == m_connectedClients.end())
		{
			continue;
		}
		DisconnectConnection dc;
		sf::Packet disconnectPacket;
		disconnectPacket << dc;
		m_commsSocket.send(disconnectPacket, k.ip, k.port);
		m_connectedClients.erase(r);
	}
}

int ServerPoll::handleReceived()
{
	InboundMessage inbound;
	const auto socketResult = m_commsSocket.receive(inbound.receivedPacket, inbound.senderIp, inbound.senderPort);

	// Socket isn't ready to receive so we'll exit

	switch (socketResult)
	{
	case sf::Socket::NotReady:
		return EXIT_SUCCESS;
		break;

	case sf::Socket::Done:
		processInboundMessage(inbound);
		break;

	case sf::Socket::Error:
	default:
		return EXIT_FAILURE;
		break;

	}

	//Thus we must be on socket done 

	return EXIT_SUCCESS;
}

void ServerPoll::processQueue()
{
	sf::Packet out;
	while (!m_messages.empty())
	{
		auto& front = m_messages.front();
		out.clear();
		m_messages.pop();


		switch (front.second->type)
		{
		case MessageType::Move:
			for (auto& c : m_connectedClients)
			{
				// if it's the client that's the argument skip, it knows its moved
				if (&c == front.first)
				{
					continue;
				}
				out << *(static_cast<MoveInWorld*>(front.second));
				m_commsSocket.send(out, c.ip, c.port);
				out.clear();

			}
			auto p = static_cast<MoveInWorld*>(front.second);
			KFREE(p);
			break;
		}

		//toClear.push_back(p);

	}
}

void ServerPoll::processInboundMessage(InboundMessage& message)
{
	const MessageType MessageType = getMessageType(message);

	switch (MessageType)
	{
	case MessageType::KeepAlive:
	{
		//Krawler::KPrintf(L"Keep alive from %s:%d\n", TO_WSTR(message.senderIp.toString()).c_str(), (int)(message.senderPort));
		auto clientIt = findGameClientFromInbound(message);
		if (clientIt != m_connectedClients.end())
		{
			clientIt->lastPollTime = system_clock::now();
			KeepAlive ka;
			message.receivedPacket >> ka;
			ka.timeStamp = timestamp();
			ka.message = "DONE";
			sf::Packet reply;
			reply << ka;
			m_commsSocket.send(reply, message.senderIp, message.senderPort);
		}
		else
		{
			auto wstr = TO_WSTR(message.senderIp.toString());
			Krawler::KPrintf(L"Unrecognised client %s:%d tried to send keep alive, packet dropped\n", wstr.c_str(), (int)message.senderPort);
		}
		break;
	}

	case MessageType::Establish:
		handleEstablish(message);
		break;
	case MessageType::Move:
		handleMove(message);
		break;
	}
}

MessageType ServerPoll::getMessageType(InboundMessage& message)
{
	const char* pPacketData = (char*)(message.receivedPacket.getData());
	assert(pPacketData);
	const int type = (pPacketData[0] << 24) | (pPacketData[1] << 16) | (pPacketData[2] << 8) | (pPacketData[3]);
	return static_cast<MessageType>(type);
}

void ServerPoll::handleEstablish(InboundMessage& message)
{
	EstablishConnection estab;
	message.receivedPacket >> estab;
	sf::Packet reply;

	auto foundClient = findGameClientFromInbound(message);
	if (foundClient != m_connectedClients.end())
	{
		foundClient->bStillConnected = true;
		foundClient->lastPollTime = system_clock::now();
		return;
	}

	if (estab.clientVersion == EXPECTED_VERSION_NUM)
	{
		GameClient gc(message.senderIp, message.senderPort);
		gc.bConnectionEstablished = true;
		gc.lastPollTime = system_clock::now();
		gc.bStillConnected = true;
		m_connectedClients.push_back(gc);
		Krawler::KPrintf(L"New client %s:%d has established a connection\n", TO_WSTR(message.senderIp.toString()).c_str(), (int)message.senderPort);
		reply << 1;
	}
	else
	{
		reply << 0;
	}
	m_commsSocket.send(reply, message.senderIp, message.senderPort);
}

void ServerPoll::handleMove(InboundMessage& message)
{
	if (m_connectedClients.size() < 1)
	{
		return;
	}

	auto gc = findGameClientFromInbound(message);
	if (gc == m_connectedClients.end())
	{
		auto wstr = TO_WSTR(message.senderIp.toString());
		Krawler::KPrintf(L"Unrecognised client %s:%d tried to send keep alive, packet dropped\n", wstr.c_str(), (int)message.senderPort);
		return;
	}

	ServerClientMessage* miw = new MoveInWorld;
	message.receivedPacket >> static_cast<MoveInWorld&>(*miw);
	m_messages.push(MessageSenderPair(&(*gc), miw));
}

std::vector<GameClient>::iterator ServerPoll::findGameClientFromInbound(InboundMessage& message)
{
	return std::find_if(m_connectedClients.begin(), m_connectedClients.end(), [&message](const GameClient& gc)
		{
			return gc.ip == message.senderIp && gc.port == message.senderPort;
		});
}
