#pragma once 

#include <Krawler.h>
#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

#include "ServerPackets.hpp"
#include "NetworkUtils.hpp"
#include <SFML/System/Clock.hpp>

using atombool = std::atomic_bool;

enum class NetworkNodeType : Krawler::int8
{
	Host = 0,
	Client
};

enum class LobbyState : Krawler::int32
{
	None = -1,
	ClientConnecting,
	ClientConnected,
	ClientDisconnected,
	HostWaiting,
	HostCancel,
	InGame
};

class SockSmeller
{
public:
	using Subscriber = std::function<void(ServerClientMessage*)>;

	~SockSmeller();

	static SockSmeller& get()
	{
		static SockSmeller& s = *new SockSmeller;
		return s;
	};

	bool isSetup() const;
	bool setupAsClient(const sf::IpAddress& ip, Krawler::uint16 port);
	bool setupAsHost(Krawler::uint16 port, Krawler::uint32 playerCount);
	void tearDown();
	void subscribeToMessageType(MessageType type, Subscriber& s);
	std::wstring getDisplayName() const { return m_displayName; }
	void setDisplayName(const std::wstring& displayName) { m_displayName = displayName; }

	bool isClientConnectionEstablished() const { return m_bConnEstablished.load(); }
	std::vector<std::wstring> getConnectedUserDisplayNames();

	void setLobbyState(LobbyState lobbyState) { m_lobbyState = lobbyState; }
	LobbyState getLobbyState() const { return m_lobbyState; }

private:

	SockSmeller();

	struct ConnectedClient
	{
		bool operator ==(const ConnectedClient& c)
		{
			return c.displayName == displayName && c.port == port && c.ip == ip;
		}

		sf::IpAddress ip;
		Krawler::uint16 port;
		Krawler::int64 lastTimestamp;
		std::wstring displayName;
	};

	void runSockSmeller();
	void runClient();
	void runHost();

	void hostCheckForDeadClients();

	void receiveHostPacket(sf::Packet& p, sf::IpAddress remoteIp, Krawler::uint16 remotePort);
	void receiveClientPacket(sf::Packet& p, sf::IpAddress remoteIp, Krawler::uint16 remotePort);

	void replyEstablishHost(const EstablishConnection& establish, const ConnectedClient& conClient);
	void hostSendDisconnect(const ConnectedClient& c);

	void clientSendKeepAlive();

	void hostSendUpdatedNameList();

	MessageType getMessageTypeFromPacket(const sf::Packet& p) const;
	ConnectedClient* getConnectedClient(const sf::IpAddress& ip, Krawler::uint16 port);

	const Krawler::uint16 SEND_PORT = 32001U;
	const float KeepAliveTime = 5.0f; // seconds
	const float ServerReplyTime = 10.0f; // seconds
	const float HostMaxDelta = 12.0f;
	bool m_bIsSetup = false;

	bool m_bReplyCountdownReset = false;
	bool m_bKeepAliveSent = false;
	sf::Clock m_keepAliveClock;
	sf::Clock m_keepAliveReplyClock;

	Krawler::uint32 m_hostLobbyplayerCount = 1;

	sf::IpAddress m_outboundIp;
	Krawler::uint16 m_outboundPort;
	Krawler::uint16 m_inboundPort;

	sf::UdpSocket m_hostSocket; // host
	sf::UdpSocket m_clientSocket; // client

	NetworkNodeType m_nodeType = NetworkNodeType::Client;
	LobbyState m_lobbyState = LobbyState::None;

	sf::String m_displayName;

	std::thread m_updateThread;

	atombool m_bIsRunning = false;
	atombool m_bConnEstablished = false;

	std::deque<ConnectedClient> m_connectedClients;
	std::map<MessageType, std::vector<Subscriber>> m_subscribersMap;

	sf::Clock m_clientEstablishClock;

	std::mutex m_connectedClientMutex;
};