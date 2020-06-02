#include <GameSetup.h>

// STD LIB
#include <chrono>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <DbgImgui.h>

// components
#include "GodDebugComp.h"
#include "Components\KCTileMap.h"
#include "PlayerLocomotive.h"
#include "PlayerRenderableComp.h"
#include "Camera.h"
#include "Spawner.h"
#include "ServerPackets.h"
#include "NetworkComms.h"

#include <SFML\Graphics\Text.hpp>

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;

using namespace std;

GameSetup::GameSetup()
{
	NetworkComms::get();
	createGod();
	createMap();
	createPlayer();
	createNetworkedPlayers();
#if RUN_SERVER
	createServer();
#endif
}

GameSetup::~GameSetup()
{
	auto p = &NetworkComms::get();
	p->closeComms();
	KFREE(p);


#if RUN_SERVER
	m_serverPoll.closeServer();
	chrono::milliseconds s(0);
	do
	{
		this_thread::sleep_for(chrono::milliseconds(5));
		s += chrono::milliseconds(5);
	} while (!m_serverPoll.hasFinishedClosingConnections());
	auto i = (int)s.count();
	KPrintf(L"I slept for %d ms\n", i);
	m_serverPollThread.join();
#endif
}

void GameSetup::tick()
{
	if (m_bShouldSpawnNetworkedPlayer)
	{
		while (!m_toSpawnQueue.empty() && m_networkedPlayerIdx < MAX_NETWORKED_PLAYERS)
		{
			auto pFont = ASSET().getFont(L"Seriphim");
			MoveInWorld miw = m_toSpawnQueue.front();

			auto& networkedPlayer = m_networkedPlayers[m_networkedPlayerIdx];
			networkedPlayer.pEntity->setActive(true);
			networkedPlayer.pEntity->getTransform()->setPosition(miw.playerPosition);
			networkedPlayer.playerName = TO_WSTR(miw.playerName);
			networkedPlayer.bSpawned = true;
			networkedPlayer.pPlayerNameText = new sf::Text(miw.playerName, *pFont);
			networkedPlayer.pPlayerNameText->setCharacterSize(16);

			const Vec2f halfBounds(networkedPlayer.pPlayerNameText->getGlobalBounds().width / 2, networkedPlayer.pPlayerNameText->getGlobalBounds().height);
			networkedPlayer.pPlayerNameText->setOrigin(halfBounds);

			networkedPlayer.pPlayerNameText->setPosition(miw.playerPosition - Vec2f(0,
				networkedPlayer.pEntity->getComponent<KCAnimatedSprite>()->getOnscreenBounds().height / 2.0f - halfBounds.y));
			GET_APP()->getRenderer()->addDebugShape(networkedPlayer.pPlayerNameText);

			m_bShouldSpawnNetworkedPlayer = false;
			++m_networkedPlayerIdx;
			m_toSpawnQueue.pop();
		}
	}


	lock_guard<mutex>lock(m_spawnInMutex);
	for (auto& np : m_networkedPlayers)
	{
		// if positions to move
		// and we're not moving
		// and we're spawned in
		if (np.positions.size() > 1 && np.bSpawned && !np.bIsMoving)
		{
			np.lastPos = np.positions.front();
			np.lastTimestamp = np.timeStamps.front();
			np.positions.pop();
			np.timeStamps.pop();

			np.bIsMoving = true;
		}
		else if (np.positions.size() == 1 && np.bSpawned && !np.bIsMoving)
		{
			if (np.positions.front() == np.pEntity->getTransform()->getPosition())
			{
				np.positions.pop();
				np.timeStamps.pop();
			}
			else
			{
				np.pEntity->getTransform()->setPosition(np.positions.front());
			}
		}

		if (np.bIsMoving)
		{
			np.lerpT += GET_APP()->getDeltaTime();
			float dt = (float)(np.timeStamps.front() - np.lastTimestamp);
			//dt = (float)(np.timeStamps.front() - np.lastTimestamp);

			dt /= 1000;
			if (np.lerpT <= dt)
			{
				Vec2f lerped = Maths::Lerp(np.lastPos, np.positions.front(), np.lerpT / dt);
				np.pEntity->getTransform()->setPosition(lerped);
			}
			else
			{
				np.lerpT = 0.0f;
				np.bIsMoving = false;
			}
		}

		if (np.bSpawned)
		{
			const Vec2f halfBounds(np.pPlayerNameText->getGlobalBounds().width, np.pPlayerNameText->getGlobalBounds().height);
			np.pPlayerNameText->setPosition(np.pEntity->getTransform()->getPosition() - Vec2f(0, 16));
		}
	}
}

void GameSetup::createGod()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"God");
	entity->addComponent(new imguicomp(entity));
	entity->addComponent(new GodDebugComp(entity, this));
}

void GameSetup::createMap()
{
	const auto LevelName = L"new_assets_map";
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"Map");
	entity->addComponent(new KCTileMapSplit(entity, LevelName));

	//entity->getTransform()->setScale(2, 2);
	entity->addComponent(new Spawner(entity, Vec2f(Maths::RandFloat(100, 200), Maths::RandFloat(100, 200))));

	m_blockedMap.setup(LevelName, entity);
}

void GameSetup::createPlayer()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"Player");
	entity->addComponent(new PlayerLocomotive(entity));
	entity->addComponent(new PlayerRenderableComp(entity));
	entity->addComponent(new Camera(entity));
}

void GameSetup::createNetworkedPlayers()
{
	for (auto& np : m_networkedPlayers)
	{
		np.pEntity = GET_SCENE()->addEntityToScene();
		np.pEntity->getTransform()->setOrigin(16, 16);
		np.pEntity->addComponent(new PlayerRenderableComp(np.pEntity));
		np.pEntity->setActive(false);
	}
	NetworkComms::get().subscribeToPacketType(MessageType::Move, &m_func);
}

#if RUN_SERVER
void GameSetup::createServer()
{
	if (!m_serverPoll.loadServer())
	{
		return;
	}

	m_serverPollThread = std::thread(&ServerPoll::runServer, &m_serverPoll);
}
#endif

void GameSetup::handleMoveInWorld(ServerClientMessage* pMessage)
{
	const MoveInWorld& miw = (*static_cast<MoveInWorld*>(pMessage));

	lock_guard<mutex>lock(m_spawnInMutex);

	auto b = begin(m_networkedPlayers);
	auto e = end(m_networkedPlayers);
	auto result = find_if(b, e, [&miw](const NetworkedPlayer& np) -> bool
		{
			return np.playerName == TO_WSTR(miw.playerName);
		});

	if (result == e)
	{
		m_bShouldSpawnNetworkedPlayer = true;
		m_toSpawnQueue.push(miw);
	}
	else
	{
		result->positions.push(miw.playerPosition);
		result->timeStamps.push(miw.timeStamp);
		//m_toMove.push(pair<Vec2f, NetworkedPlayer*>(miw.playerPosition, result));
	}
}
