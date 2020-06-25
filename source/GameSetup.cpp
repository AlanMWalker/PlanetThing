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

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;

using namespace std;

using namespace Blackboard;

static 	KCColliderBaseCallback cb = [](const KCollisionDetectionData& d)
{
	d.entityA->getComponent<KCSprite>()->setColour(Colour::Magenta);
	d.entityB->getComponent<KCSprite>()->setColour(Colour::Magenta);
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

	const uint32 TotalCelestial = PLANETS_COUNT + OBJECTS_COUNT;
	auto scene = GET_SCENE_NAMED(Blackboard::GameScene);
	auto created = scene->addMultipleEntitiesToScene(TotalCelestial, m_entities);

	KCHECK(created);

	for (int i = 0; i < TotalCelestial; ++i)
	{
		CelestialBody* celestial = nullptr;
		if (i < PLANETS_COUNT)
		{
			celestial = new CelestialBody(m_entities[i],
				CelestialBody::BodyType::Planet,
				*m_pPath

			);
		}
		else
		{
			celestial = new CelestialBody(m_entities[i],
				CelestialBody::BodyType::Satellite,
				*m_pPath
			);
		}

		KCHECK(celestial);

		m_entities[i]->addComponent(celestial);
		m_newton.addCelestialBody(*celestial);
	}

	GET_APP()->getRenderer()->addDebugShape(&line);
	GET_APP()->getRenderer()->showDebugDrawables(true);
	GET_APP()->getRenderer()->setSortType(Renderer::KRenderSortType::LayerSort);
	line.setCol(Colour::Magenta);


	m_pBackground = scene->addEntityToScene();
	m_pBackground->addComponent(new KCSprite(m_pBackground, Vec2f(GET_APP()->getWindowSize())));


	m_playerController = new LocalPlayerController(m_entities[0]->getComponent<CelestialBody>());

	auto testMoon = scene->addEntityToScene();
	auto celestial = new CelestialBody(testMoon, CelestialBody::BodyType::Moon, *m_pPath,
		m_entities[1]->getComponent<CelestialBody>());
	testMoon->addComponent(celestial);
	m_newton.addCelestialBody(*celestial);
	return KInitStatus::Success;
}

void GameSetup::onEnterScene()
{
	m_pBackgroundShader = ASSET().getShader(L"heatmap");
	//m_pBackground->getComponent<KCRenderableBase>()->setShader(m_pBackgroundShader);
	m_pBackground->getComponent<KCRenderableBase>()->setRenderLayer(-1);
	m_pBackground->getComponent<KCSprite>()->setColour(Colour::Black);


	m_defaultView = GET_APP()->getRenderWindow()->getView();
	//m_defaultView.setCenter(m_entities[0]->m_pTransform->getPosition());
	GET_APP()->getRenderWindow()->setView(m_defaultView);


	std::vector<CelestialBody*> planetsFound;
	int count = 0;
	for (auto e : m_entities)
	{
		auto celestial = e->getComponent<CelestialBody>();
		if (celestial)
		{
			if (celestial->getBodyType() == CelestialBody::BodyType::Planet)
			{
				if (count > m_aiCount)
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

void GameSetup::tick()
{
	static int idx = PLANETS_COUNT;
	if (KInput::MouseJustPressed(KMouseButton::Right))
	{
		if (idx < m_entities.size())
		{
			m_entities[idx]->getComponent<CelestialBody>()->spawnAtPoint(KInput::GetMouseWorldPosition());
			++idx;
		}

	}
	static bool bTailObject = false;
	static bool bHasZoomed = false;
	static uint32 objIdx = 0;

	if (bTailObject)
	{
		if (objIdx + (unsigned)(PLANETS_COUNT) >= m_entities.size())
		{
			objIdx = 0;
		}

		if (m_entities[(unsigned)(PLANETS_COUNT)+objIdx]->isActive())
		{
			auto& thing = m_entities[(unsigned)(PLANETS_COUNT)+objIdx];
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
	m_entities.clear();
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
