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

	// No gravity pls 
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
	setupLevel();



	m_defaultView = GET_APP()->getRenderWindow()->getView();
	//m_defaultView.setCenter(m_entities[0]->m_pTransform->getPosition());
	GET_APP()->getRenderWindow()->setView(m_defaultView);
}

void GameSetup::tick()
{
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

	bool bDidAllocate = scene->addMultipleEntitiesToScene(AI_PLANETS_COUNT, m_aiPlanets);
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

	bDidAllocate = scene->addMultipleEntitiesToScene(NETWORKED_PLANETS_COUNT, m_networkedPlanets);
	KCHECK(bDidAllocate);
	for (auto& np : m_networkedPlanets)
	{
		auto celestial = new CelestialBody(np,
			CelestialBody::BodyType::Planet,
			*m_pPath

		);
		KCHECK(celestial);
		np->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);
	}

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


	bDidAllocate = scene->addMultipleEntitiesToScene(MOON_COUNT, m_moons);
	KCHECK(bDidAllocate);
	for (auto& moon : m_moons)
	{
		auto celestial = new CelestialBody(moon,
			CelestialBody::BodyType::Satellite,
			*m_pPath

		);
		KCHECK(celestial);
		moon->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);
	}
}

void GameSetup::setupLevel()
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
	else
	{
		for (auto& ai : m_aiPlanets)
		{
			ai->getComponent<CelestialBody>()->setInActive();
		}

		for (auto np : m_networkedPlanets)
		{
			auto celestial = np->getComponent<CelestialBody>();
			if (celestial)
			{
				if (celestial->getBodyType() == CelestialBody::BodyType::Planet)
				{
					if (count > m_aiCount) // needs to be network count instead
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


	const float boundRadius = Blackboard::PLANET_RADIUS * 3.5f;
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
		return Vec2f(RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().x - boundRadius * 2),
			RandFloat(boundRadius, GET_APP()->getRenderWindow()->getSize().y - boundRadius * 2));
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

	for (uint32 i = 0; i < planetsFound.size(); ++i)
	{
		planetsFound[i]->spawnAtPoint(points[i]);
	}
}

