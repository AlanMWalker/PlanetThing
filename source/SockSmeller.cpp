#include "SockSmeller.hpp"
#include <future>

using namespace Krawler;

SockSmeller::~SockSmeller()
{
}

bool SockSmeller::isSetup() const
{
	return m_bIsSetup;
}

bool SockSmeller::setupAsClient(const sf::IpAddress& ip, uint16 port)
{
	m_outboundPort = port;
	m_outboundIp = ip;
	m_nodeType = NetworkNodeType::Client;

	auto status = m_clientSocket.bind(SEND_PORT);
	m_clientSocket.setBlocking(false);
	if (status == sf::Socket::Status::Error)
	{
		KPrintf(L"Failed to bind to local port of %hu \n", SEND_PORT);
		return false;
	}

	runSockSmeller();
	return true;
}

bool SockSmeller::setupAsHost(uint16 port)
{
	m_inboundPort = port;
	m_nodeType = NetworkNodeType::Host;

	auto status = m_hostSocket.bind(port);
	if (status == sf::Socket::Status::Error)
	{
		KPrintf(L"Failed to bind to host port of %hu \n", port);
		return false;
	}
	m_hostSocket.setBlocking(false);

	runSockSmeller();
	return true;
}

void SockSmeller::tearDown()
{
	if (!m_bIsRunning)
	{
		if (!m_updateThread.joinable())
		{
			return;
		}
	}


	m_bIsRunning = false;
	//m_updateThread.de

	m_updateThread.join();
	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		m_hostSocket.unbind();
		m_inboundPort = 0;
		break;
	default:
	case NetworkNodeType::Client:
		m_clientSocket.unbind();
		m_outboundPort = 0;
		break;
	}
}

void SockSmeller::subscribeToMessageType(MessageType type, Subscriber& s)
{
	m_subscribersMap[type].push_back(s);
}

std::vector<std::wstring> SockSmeller::getConnectedUserDisplayNames()
{
	std::vector<std::wstring> names;
	if (m_nodeType == NetworkNodeType::Host)
	{
		std::lock_guard<std::mutex> guard(m_connectedClientMutex);

		names.reserve(m_connectedClients.size());
		for (auto& c : m_connectedClients)
		{
			names.push_back(c.displayName);
		}
	}
	return names;
}

SockSmeller::SockSmeller()
	: m_outboundPort(0), m_inboundPort(0)
{
}

void SockSmeller::runSockSmeller()
{
	m_bIsRunning = true;
	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		m_updateThread = std::thread(&SockSmeller::runHost, this);
		break;
	default:
	case NetworkNodeType::Client:
		m_updateThread = std::thread(&SockSmeller::runClient, this);

		break;
	}
}

void SockSmeller::runClient()
{
	EstablishConnection e;
	e.timeStamp = timestamp();
	e.clientVersion = "0.3.0";
	e.displayName = m_displayName.toAnsiString();

	sf::Packet p;
	p << e;
	m_clientSocket.send(p, m_outboundIp, m_outboundPort);
	m_clientEstablishClock.restart();
	m_keepAliveClock.restart();
	m_keepAliveReplyClock.restart();

	while (m_bIsRunning)
	{
		//Process receive messages
		sf::IpAddress remoteIp;
		uint16 remotePort = 0u;

		sf::Socket::Status status = m_clientSocket.receive(p, remoteIp, remotePort);

		if (m_clientEstablishClock.getElapsedTime() > sf::seconds(2.0f) && !m_bConnEstablished)
		{
			p.clear();
			p << e;
			m_clientSocket.send(p, m_outboundIp, m_outboundPort);
			m_clientEstablishClock.restart();
		}

		if (m_keepAliveClock.getElapsedTime().asSeconds() > KeepAliveTime)
		{
			if (!m_bKeepAliveSent)
			{
				KPRINTF("Sending keep alive\n");
				m_keepAliveClock.restart();
				clientSendKeepAlive();
				m_bKeepAliveSent = true;

				m_keepAliveReplyClock.restart();
				m_bReplyCountdownReset = true;
			}
			else
			{

				if (m_keepAliveReplyClock.getElapsedTime().asSeconds() > ServerReplyTime)
				{
					KPRINTF("Seems like the server hasn't been replying to my keep alives...\n");
					m_bIsRunning = false;
				}
			}
		}

		switch (status)
		{
		case sf::Socket::Done:
			receiveClientPacket(p, remoteIp, remotePort);
			break;
		case sf::Socket::NotReady:
		case sf::Socket::Partial:
		case sf::Socket::Disconnected:
			break;

		case sf::Socket::Error:
		default:
			KPrintf(L"Error found when listening on host port %hu, stopping host\n", SEND_PORT);
			m_bIsRunning = false;
			break;
		}
		// Send new packets
	}
}

void SockSmeller::runHost()
{
	while (m_bIsRunning)
	{
		std::lock_guard<std::mutex>guard(m_connectedClientMutex);
		//Process receive messages
		sf::Packet p;
		sf::IpAddress remoteIp;
		uint16 remotePort = 0u;
		sf::Socket::Status status = m_hostSocket.receive(p, remoteIp, remotePort);

		switch (status)
		{
		case sf::Socket::Done:
			receiveHostPacket(p, remoteIp, remotePort);
			break;
		case sf::Socket::NotReady:
			break;
		case sf::Socket::Partial:
		case sf::Socket::Disconnected:
		case sf::Socket::Error:
		default:
			KPrintf(L"Error found when listening on host port %hu, stopping host\n", m_inboundPort);
			m_bIsRunning = false;
			break;
		}

		hostCheckForDeadClients();
		// Send new packets
	}
	KPrintf(L"Bye..\n");
}

void SockSmeller::hostCheckForDeadClients()
{
	const int64 MaxTimeDelta = sf::seconds(HostMaxDelta).asMilliseconds();
	auto currentTimestamp = timestamp();
	std::deque<std::deque<ConnectedClient>::iterator> toRemove;
	for (auto it = m_connectedClients.begin(); it != m_connectedClients.end(); ++it)
	{
		if (currentTimestamp - (*it).lastTimestamp > MaxTimeDelta)
		{
			toRemove.push_back(it);
		}
	}

	for (auto& i : toRemove)
	{
		KPrintf(L"Client %s:%hu has timed out...\n", TO_WSTR(i->ip.toString()).c_str(), i->port);
		hostSendDisconnect(*i);
		m_connectedClients.erase(i);
	}
}

void SockSmeller::receiveHostPacket(sf::Packet& p, sf::IpAddress remoteIp, uint16 remotePort)
{
	const MessageType type = getMessageTypeFromPacket(p);

	ConnectedClient conClient;
	conClient.ip = remoteIp;
	conClient.port = remotePort;

	switch (type)
	{
	case MessageType::Establish:
	{
		EstablishConnection con;
		p >> con;

		conClient.lastTimestamp = timestamp();
		conClient.displayName = TO_WSTR(con.displayName);
		replyEstablishHost(con, conClient);
	}
	break;
	case MessageType::KeepAlive:
	{
		KeepAlive ka;
		p >> ka;
		KPrintf(L"Inbound keep alive from %s : %hu \n", &TO_WSTR(remoteIp.toString())[0], remotePort);
		// we receieved a keep alive, so we'll send back 
		// a keep alive
		auto client = getConnectedClient(remoteIp, remotePort);
		if (client)
		{
			client->lastTimestamp = ka.timeStamp;
			sf::Packet reply;
			ka.timeStamp = timestamp();
			reply << ka;
			m_hostSocket.send(reply, remoteIp, remotePort);
		}
	}
	break;
	case MessageType::None:
	default:
		KPRINTF("Unrecognised message sent to host..\n");
		break;
	}
}

void SockSmeller::receiveClientPacket(sf::Packet& p, sf::IpAddress remoteIp, Krawler::uint16 remotePort)
{
	const MessageType type = getMessageTypeFromPacket(p);

	KCHECK(remoteIp == m_outboundIp);
	KCHECK(remotePort == m_outboundPort);

	if (remoteIp != m_outboundIp || remotePort != m_outboundPort)
	{
		return;
	}

	switch (type)
	{
	case MessageType::Establish:
	{
		if (m_subscribersMap.count(MessageType::Establish) > 0)
		{
			for (auto s : m_subscribersMap[MessageType::Establish])
			{
				EstablishConnection e;
				p >> e;
				m_bConnEstablished = true;
				s(&e);
			}
		}
	}
	break;
	case MessageType::KeepAlive:
	{
		KPrintf(L"Server replied to keep alive\n");
		if (m_bKeepAliveSent)
		{
			if (m_bReplyCountdownReset)
			{
				m_bKeepAliveSent = false;
				m_bReplyCountdownReset = false;
				//m_keepAliveClock.restart();
				//m_keepAliveReplyClock.restart();
			}
		}
	}
	break;
	case MessageType::Disconnect:
	{
		if (m_subscribersMap.count(MessageType::Establish) > 0)
		{
			for (auto s : m_subscribersMap[MessageType::Disconnect])
			{
				DisconnectConnection dc;
				p >> dc;
				m_bConnEstablished = true;
				s(&dc);
			}
		}
	}
	break;

	case MessageType::LobbyNameList:
	{
		for (auto s : m_subscribersMap[MessageType::LobbyNameList])
		{
			LobbyNameList lnl;
			p >> lnl;
			s(&lnl);
		}
	}
	break;

	case MessageType::None:
	default:
		break;
	}
}

void SockSmeller::replyEstablishHost(const EstablishConnection& establish, const ConnectedClient& conClient)
{
	auto client = getConnectedClient(conClient.ip, conClient.port);
	if (client)
	{// Already connected, ignore this
		return;
	}
	m_connectedClients.push_back(conClient);

	EstablishConnection e(establish);
	e.timeStamp = timestamp();
	sf::Packet p;
	p << e;
	m_hostSocket.send(p, conClient.ip, conClient.port);

	// Send all clients an updated name list
	for (auto c : m_connectedClients)
	{
		LobbyNameList lnl;
		lnl.timeStamp = timestamp();

		lnl.nameList += TO_ASTR(m_displayName);
		lnl.nameList += ',';

		for (int i = 0; i < m_connectedClients.size(); ++i)
		{
			if (c == m_connectedClients[i])
			{
				continue;
			}

			lnl.nameList += TO_ASTR(m_connectedClients[i].displayName);
			if (i < m_connectedClients.size() - 1)
			{
				lnl.nameList += ',';
			}
		}

		sf::Packet p;
		p << lnl;
		m_hostSocket.send(p, c.ip, c.port);
	}
}

void SockSmeller::hostSendDisconnect(const ConnectedClient& c)
{
	DisconnectConnection dc;
	sf::Packet p;
	p << dc;
	m_hostSocket.send(p, c.ip, c.port);
}

void SockSmeller::clientSendKeepAlive()
{
	KeepAlive ka;
	ka.timeStamp = timestamp();
	sf::Packet p;
	p << ka;
	m_clientSocket.send(p, m_outboundIp, m_outboundPort);
}

MessageType SockSmeller::getMessageTypeFromPacket(const sf::Packet& p) const
{
	const char* const asBytes = (char*)p.getData();
	KCHECK(asBytes);
	if (asBytes == nullptr)
	{
		return MessageType::None;
	}
	return (MessageType)((asBytes[0] << 24) | (asBytes[1] << 16) | (asBytes[2] << 8) | (asBytes[3]));
}

SockSmeller::ConnectedClient* SockSmeller::getConnectedClient(const sf::IpAddress& ip, Krawler::uint16 port)
{
	auto result = std::find_if(m_connectedClients.begin(), m_connectedClients.end(), [&ip, &port](const ConnectedClient& test) -> bool
		{
			return port == test.port && ip == test.ip;
		});

	if (result == m_connectedClients.end())
	{
		return nullptr;
	}

	return &(*result);
}