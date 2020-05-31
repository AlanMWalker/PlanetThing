#pragma once 

// std lib shtuff
#include <thread>
#include <deque>
#include <mutex>
#include <set>
#include <atomic>
#include <functional>

// sf includes
#include <SFML/Network.hpp>


//engine includes
#include <Krawler.h>

// server includes
#include "ServerPackets.h"

// game includes

struct NetworkInfo
{
	sf::IpAddress serverIp;
	unsigned short serverPort;
	unsigned short myPort;
	std::string playerName;
};

class NetworkComms
{
public:
	static NetworkComms& get()
	{
		static NetworkComms* instance = new NetworkComms();
		return *instance;
	}
	~NetworkComms();

	void spawnIn(const Krawler::Vec2f& pos);
	void moveInWorld(const Krawler::Vec2f& pos);
	void subscribeToPacketType(MessageType type, std::function<void(ServerClientMessage*)>*);
	void setPlayerName(const std::wstring& name) { m_networkInfo.playerName = TO_ASTR(name); }
	const std::atomic_bool& isSpawnedIn() const { return m_bDidSpawnInWorld; }

	void closeComms();

	sf::Time getServerResponseTime() const { return sf::milliseconds(m_serverResponseTime); }

private:

	NetworkComms();

	void loadNetworkConf();
	void connectToServer();
	void networkPollLoop();
	void handleReplies(sf::Packet& p);
	void processQueue();

	std::thread m_networkPollThread;
	std::mutex m_socketMutex;
	std::mutex m_queueMutex;
	std::atomic_bool m_bConnEstablished = false;
	std::atomic_bool m_bDidSpawnInWorld = false;
	std::atomic_bool m_bCommsAlive = true;

	NetworkInfo m_networkInfo;
	sf::UdpSocket m_connSocket;

	std::map<MessageType, std::vector<std::function<void(ServerClientMessage*)>*>> m_typeSubscribers;
	std::deque<sf::Packet> m_messageQueue;

	long long m_lastSentTimestamp = 0;
	std::atomic_llong m_serverResponseTime;

	// TODO move to a blackboard class 
	const std::wstring GAME_CLIENT_VERSION{ L"0.0.1" };
	const int MAX_RETRY_TIME = 15; // in seconds

};

