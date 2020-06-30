#pragma once 

#include <Krawler.h>
#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include <deque>
#include <functional>

#include "ServerPackets.hpp"
#include "NetworkUtils.hpp"
#include <SFML/System/Clock.hpp>

using atombool = std::atomic_bool;

enum class NetworkNodeType : Krawler::int8
{
	Host = 0,
	Client
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
	bool setupAsHost(Krawler::uint16 port);
	void tearDown();
	void subscribeToMessageType(MessageType type, Subscriber& s);

private:

	SockSmeller();

	struct ConnectedClient
	{
		sf::IpAddress ip;
		Krawler::uint16 port;
		Krawler::int64 lastTimestamp;
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



	sf::IpAddress m_outboundIp;
	Krawler::uint16 m_outboundPort;
	Krawler::uint16 m_inboundPort;

	sf::UdpSocket m_hostSocket; // host
	sf::UdpSocket m_clientSocket; // client

	NetworkNodeType m_nodeType = NetworkNodeType::Client;

	std::thread m_updateThread;

	atombool m_bIsRunning = false;
	atombool m_bConnEstablished = false;

	std::deque<ConnectedClient> m_connectedClients;
	std::map<MessageType, std::vector<Subscriber>> m_subscribersMap;

	sf::Clock m_clientEstablishClock;
};
