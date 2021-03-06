#include "GameSetup.hpp"
// STD LIB
#include <chrono>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>

#include <Components/KCBody.h>
#include <Components/KCCircleCollider.h>

// components
#include "GodDebugComp.hpp"
#include "DbgImgui.hpp"
#include "CelestialBody.hpp"
#include "CPUPlayerController.hpp"
#include "Invoker.hpp"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;

using namespace std;

using namespace Blackboard;

static 	KCColliderBaseCallback cb = [](const KCollisionDetectionData& d)
{
	//d.entityA->getComponent<KCSprite>()->setColour(Colour::Magenta);
	//d.entityB->getComponent<KCSprite>()->setColour(Colour::Magenta);
};


GameSetup::GameSetup()
	: KComponentBase(GET_APP()->getSceneDirector().getSceneByName(Blackboard::GameScene)->addEntityToScene())
{
	getEntity()->addComponent(this);
}

GameSetup::~GameSetup()
{

}

KInitStatus GameSetup::init()
{
	createGod();
	GET_APP()->setPrintFPS(true);
	GET_APP()->getRenderer()->setSortType(Renderer::KRenderSortType::LayerSort);

	// No gravity please 
	m_pPhysicsWorld = &GET_APP()->getPhysicsWorld();
	m_pPhysicsWorld->setGravity(Vec2f(0.0f, 0.0f));
	m_pPhysicsWorld->setPPM(PPM);
	auto scene = GET_SCENE_NAMED(Blackboard::GameScene);

	GET_APP()->getRenderer()->addDebugShape(&line);
	GET_APP()->getRenderer()->showDebugDrawables(true);
	GET_APP()->getRenderer()->setSortType(Renderer::KRenderSortType::LayerSort);
	line.setCol(Colour::Magenta);


	m_pBackground = scene->addEntityToScene();
	m_pBackground->setTag(L"Background");
	m_pBackground->addComponent(new KCSprite(m_pBackground, Vec2f(GET_APP()->getWindowSize())));

	createCelestialBodies();

	m_playerController = new LocalPlayerController(m_pPlayerPlanet->getComponent<CelestialBody>());

	return KInitStatus::Success;
}

void GameSetup::onEnterScene()
{
	m_pBackgroundShader = ASSET().getShader(L"heatmap");
	auto backgroundTex = ASSET().getTexture(L"space_bg");
	//m_pBackground->getComponent<KCRenderableBase>()->setShader(m_pBackgroundShader);
	m_pBackground->getComponent<KCRenderableBase>()->setRenderLayer(-1);
	//m_pBackground->getComponent<KCSprite>()->setColour(Colour::Black);
	m_pBackground->getComponent<KCSprite>()->setTexture(backgroundTex);

	switch (m_gameType)
	{
	case GameSetup::GameType::Local:
		setupLevelLocal();
		break;
	case GameSetup::GameType::Networked:
		if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host)
		{
			setupLevelNetworkedHost();
		}
		else
		{
			setupLevelNetworkedClient();
		}

		break;
	default:
		// Like seriously, this should never happen..
		KCHECK(false);
		break;
	}

	m_defaultView = GET_APP()->getRenderWindow()->getView();
	//m_defaultView.setCenter(m_entities[0]->m_pTransform->getPosition());
	GET_APP()->getRenderWindow()->setView(m_defaultView);
}

void GameSetup::tick()
{
	manageGameState();

	static int idx = PLANETS_COUNT;
	if (KInput::MouseJustPressed(KMouseButton::Right))
	{
		if (idx < m_satellites.size())
		{
			m_satellites[idx]->getComponent<CelestialBody>()->spawnAtPoint(KInput::GetMouseWorldPosition());
			++idx;
		}

	}
	static bool bTailObject = false;
	static bool bHasZoomed = false;
	static uint64 objIdx = 0;

	if (bTailObject)
	{
		if (objIdx >= m_satellites.size())
		{
			objIdx = 0;
		}

		if (m_satellites[objIdx]->isActive())
		{
			auto& thing = m_satellites[objIdx];
			bHasZoomed = true;
			constexpr float ZoomAmount = 1.3f;
			auto f = KInput::GetMouseScrollDelta();
			if (f > 0.0f)
			{
				zoomAt(Vec2f(thing->m_pTransform->getPosition()), 1.0f / ZoomAmount);
			}
			else if (f < 0.0f)
			{
				zoomAt(Vec2f(thing->m_pTransform->getPosition()), ZoomAmount);
			}
			sf::View v = GET_APP()->getRenderWindow()->getView();
			v.setCenter(thing->m_pTransform->getPosition());
			GET_APP()->getRenderWindow()->setView(v);
		}

		if (KInput::JustPressed(KKey::Right))
		{
			++objIdx;
		}

		if (KInput::JustPressed(KKey::Left))
		{
			--objIdx;
		}

	}
	else
	{
		GET_APP()->getRenderWindow()->setView(m_defaultView);
	}

	if (KInput::JustPressed(KKey::Space))
	{
		bTailObject = !bTailObject;
		bHasZoomed = false;
	}

	setBackgroundShaderParams();
}

void GameSetup::fixedTick()
{
	m_newton.applyForces();
}

void GameSetup::cleanUp()
{
	m_pPlayerPlanet = nullptr;
	m_aiPlanets.clear();
	m_networkedPlanets.clear();
}

void GameSetup::setAIPlayerCount(int32 count)
{
	m_aiCount = count;
}

void GameSetup::createGod()
{
	auto entity = getEntity();
	entity->setTag(L"God");
	entity->addComponent(new Invoker(entity));
	entity->addComponent(new imguicomp(entity));
	entity->addComponent(new GodDebugComp(entity));
	m_pPath = new ProjectilePath(entity);
	entity->addComponent(m_pPath);
}

void GameSetup::zoomAt(const Krawler::Vec2f& pos, float zoom)
{
	sf::View v = GET_APP()->getRenderWindow()->getView();
	v.zoom(zoom);
	v.setCenter(pos);
	GET_APP()->getRenderWindow()->setView(v);
}

void GameSetup::setBackgroundShaderParams()
{
	/*std::vector<Vec2f> planetPositions;
	std::vector<Vec3f> planetCols;
	for (auto& so : m_spaceThings)
	{
		if (so.bIsPlanet)
		{
			const Vec2f screenPos(GET_APP()->getRenderWindow()->mapCoordsToPixel(so.pEntity->m_pTransform->getPosition()));
			planetPositions.push_back(screenPos);
			planetCols.push_back(Vec3f((float)(so.col.r) / 256.0f, (float)(so.col.g) / 256.0f, (float)(so.col.b) / 256.0f));
		}
	}

	m_pBackgroundShader->setUniform("planetCount", PLANETS_COUNT);
	m_pBackgroundShader->setUniformArray("planetPositions", &planetPositions[0], PLANETS_COUNT);
	m_pBackgroundShader->setUniformArray("planetColours", &planetCols[0], PLANETS_COUNT);

	int h = GET_APP()->getRenderWindow()->getView().getSize().y;
	m_pBackgroundShader->setUniform("windowHeight", (int)GET_APP()->getRenderWindow()->getView().getSize().y);
	m_pBackgroundShader->setUniform("colScale", colScale);*/

}

void GameSetup::createCelestialBodies()
{
	auto scene = GET_SCENE_NAMED(Blackboard::GameScene);

	// Allocate networked player satellite entities
	// and networked player controllers
	bool bDidAllocate = scene->addMultipleEntitiesToScene(AI_PLANETS_COUNT, m_networkedSatellites);
	KCHECK(bDidAllocate);

	const uint32 TotalCelestial = PLANETS_COUNT + SATELLITES_COUNT;
	//auto created = scene->addMultipleEntitiesToScene(TotalCelestial, m_entities);
	m_pPlayerPlanet = scene->addEntityToScene();
	auto celestial = new CelestialBody(m_pPlayerPlanet,
		CelestialBody::BodyType::Planet,
		*m_pPath

	);
	KCHECK(celestial);
	m_pPlayerPlanet->addComponent(celestial);
	m_newton.addCelestialBody(*celestial);

	// Allocate AI Planets
	bDidAllocate = scene->addMultipleEntitiesToScene(AI_PLANETS_COUNT, m_aiPlanets);
	KCHECK(bDidAllocate);
	for (auto& ai : m_aiPlanets)
	{
		auto celestial = new CelestialBody(ai,
			CelestialBody::BodyType::Planet,
			*m_pPath

		);
		KCHECK(celestial);
		ai->addComponent(celestial);
		new CPUPlayerController(celestial);
		m_newton.addCelestialBody(*celestial);
	}
	// Allocate networked planets
	bDidAllocate = scene->addMultipleEntitiesToScene(NETWORKED_PLANETS_COUNT, m_networkedPlanets);
	KCHECK(bDidAllocate);
	for (auto& np : m_networkedPlanets)
	{
		//Setup celestial body
		auto celestial = new CelestialBody(np,
			CelestialBody::BodyType::Planet,
			*m_pPath

		);
		KCHECK(celestial);
		np->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);

	}


	for (uint64 i = 0; i < MAX_NETWORKED; ++i)
	{
		// setup networked player controller 
		m_networkedControllers[i] = new NetworkPlayerController(m_networkedSatellites[i],
			m_networkedPlanets[i]->getComponent<CelestialBody>()
		);

		KCHECK(m_networkedControllers[i]);
		m_networkedSatellites[i]->addComponent(m_networkedControllers[i]);
	}

	// Allocate projectile satellites
	bDidAllocate = scene->addMultipleEntitiesToScene(SATELLITES_COUNT, m_satellites);
	KCHECK(bDidAllocate);
	for (auto& sat : m_satellites)
	{
		auto celestial = new CelestialBody(sat,
			CelestialBody::BodyType::Satellite,
			*m_pPath

		);
		KCHECK(celestial);
		sat->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);
	}


	// Allocate moons
	bDidAllocate = scene->addMultipleEntitiesToScene(MOON_COUNT, m_moons);
	KCHECK(bDidAllocate);
	for (auto& moon : m_moons)
	{
		auto celestial = new CelestialBody(moon,
			CelestialBody::BodyType::Moon,
			*m_pPath
		);
		KCHECK(celestial);
		moon->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);
	}
}

void GameSetup::setupLevelLocal()
{
	std::vector<CelestialBody*> planetsFound;
	int count = 0;
	planetsFound.push_back(m_pPlayerPlanet->getComponent<CelestialBody>());
	if (m_gameType == GameType::Local)
	{
		for (auto& np : m_networkedPlanets)
		{
			np->getComponent<CelestialBody>()->setInActive();
		}

		for (auto ai : m_aiPlanets)
		{
			auto celestial = ai->getComponent<CelestialBody>();
			if (celestial)
			{
				if (celestial->getBodyType() == CelestialBody::BodyType::Planet)
				{
					if (count >= m_aiCount)
					{
						celestial->setInActive();
					}
					else
					{
						planetsFound.push_back(celestial);
						++count;
					}
				}
			}
		}
	}


	const float boundRadius = Blackboard::PLANET_RADIUS * 4.0f;
	std::vector<Vec2f> points;
	points.resize(planetsFound.size());

	auto doesOverlap = [&points, &boundRadius](const Vec2f& p) -> bool
	{
		for (auto& v : points)
		{
			if (&v == &p)
			{
				continue;
			}

			const bool isBelow = GetSquareLength(v - p) < boundRadius * boundRadius;
			if (isBelow)
			{
				return true;
			}
		}
		return false;
	};

	auto randPoint = [&boundRadius]() -> Vec2f
	{
		return Vec2f(RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().x - boundRadius),
			RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().y - boundRadius));
	};

	for (auto& v : points)
	{
		v = randPoint();
	}

	for (auto& v : points)
	{
		while (doesOverlap(v))
		{
			v = randPoint();
		}
	}

	for (uint64 i = 0; i < points.size(); ++i)
	{
		planetsFound[i]->setPosition(points[i]);
	}
}

void GameSetup::setupLevelNetworkedHost()
{
	std::vector<CelestialBody*> planetsFound;
	GeneratedLevel genLevel;
	genLevel.numOfPlanets = (uint64)(m_networkedCount + 1u);
	setupNetworkedPlanetsAndSatellites(genLevel, planetsFound);

	const float boundRadius = Blackboard::PLANET_RADIUS * 4.0f;
	std::vector<Vec2f> points;
	points.resize(planetsFound.size());

	auto doesOverlap = [&points, &boundRadius](const Vec2f& p) -> bool
	{
		for (auto& v : points)
		{
			if (&v == &p)
			{
				continue;
			}

			const bool isBelow = GetSquareLength(v - p) < boundRadius * boundRadius;
			if (isBelow)
			{
				return true;
			}
		}
		return false;
	};

	auto randPoint = [&boundRadius]() -> Vec2f
	{
		return Vec2f(RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().x - boundRadius),
			RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().y - boundRadius));
	};

	for (auto& v : points)
	{
		v = randPoint();
	}

	for (auto& v : points)
	{
		while (doesOverlap(v))
		{
			v = randPoint();
		}
	}

	for (uint64 i = 0; i < points.size(); ++i)
	{
		planetsFound[i]->setPosition(points[i]);
	}

	using UUIDName = std::pair< std::string, std::string>;
	std::stack<UUIDName> uuidAndName;

	auto clientList = SockSmeller::get().getClientList();
	std::random_shuffle(clientList.begin(), clientList.end());

	for (auto c : clientList)
	{
		uuidAndName.push(UUIDName(TO_ASTR(c.uuid), TO_ASTR(c.displayName)));
	}

	// send planet mass & positions to other client
	for (uint64 i = 0; i < points.size(); ++i)
	{
		auto p = planetsFound[i];
		genLevel.masses.push_back(p->getMass());
		genLevel.positions.push_back(p->getCentre());

		if (p->getEntity() == m_pPlayerPlanet)
		{
			genLevel.names.push_back(TO_ASTR(SockSmeller::get().getDisplayName()));
			genLevel.uuids.push_back(TO_ASTR(SockSmeller::get().getMyUUID()));

		}
		else
		{
			KCHECK(!uuidAndName.empty());
			genLevel.uuids.push_back(uuidAndName.top().first);
			genLevel.names.push_back(uuidAndName.top().second);
			for (auto& c : m_networkedControllers)
			{
				if (c->getHostPlanet() == p)
				{
					c->setUUID(TO_WSTR(uuidAndName.top().first));
				}
			}
			uuidAndName.pop();
		}
	}
	genLevel.numOfPlanets = planetsFound.size();
	SockSmeller::get().hostSendGenLevel(genLevel);


	// shuffle name list and set first player 
	for (auto client : SockSmeller::get().getClientList())
	{
		m_lobbyPlayers.push_back(client.uuid);
	}
	m_lobbyPlayers.push_back(SockSmeller::get().getMyUUID());

	std::random_shuffle(m_lobbyPlayers.begin(), m_lobbyPlayers.end());
	m_currentPlayerTurnIdx = 0;

	// if I'm the first uuid in the list, then I take the first turn
	if (m_lobbyPlayers[m_currentPlayerTurnIdx] == SockSmeller::get().getMyUUID())
	{
		m_playerController->setTurnIsActive(true);
	}
}

void GameSetup::setupLevelNetworkedClient()
{
	std::vector<CelestialBody*> planetsFound;
	setupNetworkedPlanetsAndSatellites(m_genLevel, planetsFound);


	for (uint64 i = 0; i < m_genLevel.positions.size(); ++i)
	{
		planetsFound[i]->setPosition(m_genLevel.positions[i]);
		planetsFound[i]->setMass(m_genLevel.masses[i]);
		if (TO_WSTR(m_genLevel.uuids[i]) == SockSmeller::get().getMyUUID())
		{
			m_playerController->setHostPlanet(planetsFound[i]);

			// If my planet is the first in the list
			// it's my turn
			if (i == 0)
			{
				m_playerController->setTurnIsActive(true);
			}
		}

		for (auto& controller : m_networkedControllers)
		{
			if (controller->getHostPlanet() == planetsFound[i])
			{
				controller->getEntity()->setActive(true);
				controller->setUUID(TO_WSTR(m_genLevel.uuids[i]));
				if (i == 0)
				{
					controller->setTurnIsActive(true);
				}
				KPrintf(L"Networked player UUID is %s \n", TO_WSTR(m_genLevel.uuids[i]).c_str());
			}
		}
	}
}

void GameSetup::manageGameState()
{
	switch (m_gameType)
	{
	case GameSetup::GameType::Networked:
		manageNetworked();
		break;
	case GameSetup::GameType::Local:
		manageLocal();
	default:
		break;
	}
}

void GameSetup::manageNetworked()
{
	// Nothing to manage as a networked client, the host handles it all
	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
	{
		return;
	}

	// As the host
	static bool bIsFirstRun = true;
	if (bIsFirstRun)
	{
		if (m_lobbyPlayers[0] == SockSmeller::get().getMyUUID())
		{
			// No need to send network message
			// host starts
		}
		else
		{
			// Send message to the clients
			// to say which client starts
		}
		bIsFirstRun = false;
	}
}

void GameSetup::manageLocal()
{
	// STUB
}

void GameSetup::setupNetworkedPlanetsAndSatellites(const GeneratedLevel& genLevel, std::vector<CelestialBody*>& planetsFound)
{
	int count = 0;
	planetsFound.push_back(m_pPlayerPlanet->getComponent<CelestialBody>());
	for (auto& ai : m_aiPlanets)
	{
		ai->getComponent<CelestialBody>()->setInActive();
	}

	for (uint64 i = 0; i < MAX_NETWORKED; ++i)
	{
		auto networkedPlayer = m_networkedPlanets[i];
		auto celestial = networkedPlayer->getComponent<CelestialBody>();
		KCHECK(celestial);
		if (celestial)
		{
			if (celestial->getBodyType() == CelestialBody::BodyType::Planet)
			{
				// -1 for players planet
				if (count >= (genLevel.numOfPlanets - 1))
				{
					celestial->setInActive();
					m_networkedSatellites[i]->setActive(false);
					m_networkedControllers[i]->hideTargets();
				}
				else
				{
					planetsFound.push_back(celestial);
					m_networkedSatellites[i]->setActive(true);
					m_networkedControllers[i]->showTargets();
					++count;
				}
			}
		}
	}
}

