#pragma once

#define RUN_SERVER 1

#include <BlockedMap.h>
#include <SFML\Network.hpp>
#include <atomic>
#include <queue>
#include <functional>
#include <mutex>
#include <KEntity.h>
#include <thread>

#include "ServerPackets.h"

#if RUN_SERVER
#include "ServerPoll.h"
#endif

namespace sf
{
	class Text;
}

class NetworkComms;
constexpr unsigned int MAX_NETWORKED_PLAYERS{ 5 };

struct NetworkedPlayer
{
	std::wstring playerName;
	Krawler::Vec2f lastPos;
	std::queue<Krawler::Vec2f> positions;
	std::queue<long long> timeStamps;
	long long lastTimestamp;
	Krawler::KEntity* pEntity;
	sf::Text* pPlayerNameText;
	float lerpT = 0.0f;
	bool bSpawned = false;
	bool bIsMoving = false;
};


class GameSetup
{
public:

	GameSetup();
	~GameSetup();

	void tick();

	const BlockedMap& getBlockedMap() const { return m_blockedMap; }

private:

	void createGod();
	void createMap();
	void createBlockedMap();
	void createPlayer();
	void createNetworkedPlayers();

	void handleMoveInWorld(ServerClientMessage* pMessage);

#if RUN_SERVER
	void createServer();

	std::thread m_serverPollThread;
	ServerPoll m_serverPoll;
	
#endif

	BlockedMap m_blockedMap;


	NetworkedPlayer m_networkedPlayers[MAX_NETWORKED_PLAYERS];
	std::queue<MoveInWorld> m_toSpawnQueue;
	std::queue<std::pair<Krawler::Vec2f, NetworkedPlayer*>> m_toMove;

	const unsigned int MAX_CONNECTION_RETRIES = 3;

	std::atomic_bool m_bShouldSpawnNetworkedPlayer = false;
	std::mutex m_spawnInMutex;
	std::atomic_uint32_t m_networkedPlayerIdx = 0;

	std::function<void(ServerClientMessage*)> m_func = [this](ServerClientMessage* pMessage)
	{
		// Change the client network sub 
		// to take function pointers instead of this 
		handleMoveInWorld(pMessage);

	};
};