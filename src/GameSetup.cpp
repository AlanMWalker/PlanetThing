#include <GameSetup.h>

// STD LIB
#include <chrono>

// engine
#include <Krawler.h>
#include <KApplication.h>
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
	createBlockedMap();
	createPlayer();
	createNetworkedPlayers();
}

GameSetup::~GameSetup()
{
	auto p = &NetworkComms::get();
	KFREE(p);
}

void GameSetup::tick()
{
	if (m_bShouldSpawnNetworkedPlayer)
	{
		while (!m_toSpawnQueue.empty() && m_networkedPlayerIdx < MAX_NETWORKED_PLAYERS)
		{
			MoveInWorld miw = m_toSpawnQueue.front();
			m_networkedPlayers[m_networkedPlayerIdx].pEntity->setActive(true);
			m_networkedPlayers[m_networkedPlayerIdx].pEntity->getTransform()->setPosition(miw.playerPosition);
			m_networkedPlayers[m_networkedPlayerIdx].playerName = TO_WSTR(miw.playerName);
			m_networkedPlayers[m_networkedPlayerIdx].bSpawned = true;
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
		if (np.positions.size() > 0)
			KPrintf(L"Number of positions to catch up on is - %d\n", np.positions.size());

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
			/*	if (np.lastTimestamp > 0)
				{
					np.lastPos = np.pEntity->getTransform()->getPosition();

					np.bIsMoving = true;
				}*/
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
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"Map");
	entity->addComponent(new KCTileMapSplit(entity, L"test_level"));
	entity->addComponent(new Spawner(entity, Vec2f(Maths::RandFloat(100, 200), Maths::RandFloat(100, 200))));
}

void GameSetup::createBlockedMap()
{
	m_blockedMap.setup(L"test_level");
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
