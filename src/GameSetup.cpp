#include <GameSetup.h>
// STD LIB
#include <chrono>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include "DbgImgui.h"

#include <Components/KCBody.h>
#include <Components/KCCircleCollider.h>

// components
#include "GodDebugComp.h"

#include <SFML\Graphics\Text.hpp>

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;

using namespace std;

static 	KCColliderBaseCallback cb = [](const KCollisionDetectionData& d)
{
	d.entityA->getComponent<KCSprite>()->setColour(Colour::Magenta);
	d.entityB->getComponent<KCSprite>()->setColour(Colour::Magenta);
};

Colour genColour()
{
	const float r = roundf((Maths::RandFloat() * 127) + 127);
	const float g = roundf((Maths::RandFloat() * 127) + 127);
	const float b = roundf((Maths::RandFloat() * 127) + 127);

	return Colour(r, g, b, 255);
}

GameSetup::GameSetup(Krawler::KEntity* pEntity)
	: KComponentBase(pEntity)
{

}

GameSetup::~GameSetup()
{

}

KInitStatus GameSetup::init()
{
	createGod();
	GET_APP()->setPrintFPS(false);
	GET_APP()->getRenderer()->setSortType(Renderer::KRenderSortType::LayerSort);

	// No gravity pls 
	m_pPhysicsWorld = &GET_APP()->getPhysicsWorld();
	m_pPhysicsWorld->setGravity(Vec2f(0.0f, 0.0f));
	m_pPhysicsWorld->setPPM(PPM);

	for (int i = 0; i < PLANETS_COUNT; ++i)
	{
		SpaceObject so;
		so.bIsPlanet = true;
		so.mass = RandFloat(PLANET_MASS, PLANET_MASS * 2);
		so.radius = PLANET_RADIUS;
		so.pEntity = GET_SCENE()->addEntityToScene();
		so.col = genColour();
		so.pEntity->setTag(L"Planet_" + to_wstring(i + 1));


		const Vec2f bounds(2.0f * PLANET_RADIUS, 2.0f * PLANET_RADIUS);

		so.pEntity->addComponent(new KCSprite(so.pEntity, bounds));
		so.pEntity->m_pTransform->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

		so.pEntity->m_pTransform->setPosition(Vec2f(
			RandFloat(100, 1700),
			RandFloat(100, 900)
		));

		so.pEntity->addComponent(new KCCircleCollider(so.pEntity, PLANET_RADIUS));
		KBodyDef bodyDef;
		KMatDef matDef;
		matDef.restitution = 0.56f;
		bodyDef.bodyType = BodyType::Dynamic_Body;
		bodyDef.position = so.pEntity->m_pTransform->getPosition() / m_pPhysicsWorld->getPPM();
		matDef.density = so.getDensity();
		so.pPhysicsBody = new KCBody(*so.pEntity, bounds, bodyDef, matDef);
		so.pEntity->addComponent(so.pPhysicsBody);
		m_spaceThings.push_back(so);
	}

	for (int i = 0; i < OBJECTS_COUNT; ++i)
	{
		SpaceObject so;
		so.bIsPlanet = false;
		so.mass = OBJECT_MASS;
		so.radius = OBJECT_RADIUS;
		so.pEntity = GET_SCENE()->addEntityToScene();
		so.pEntity->setTag(L"Object_" + to_wstring(i + 1));
		so.col = genColour();
		const Vec2f bounds(2.0f * OBJECT_RADIUS, 2.0f * OBJECT_RADIUS);

		so.pEntity->setActive(false);
		so.pEntity->addComponent(new KCSprite(so.pEntity, bounds));
		so.pEntity->m_pTransform->setOrigin(Vec2f(OBJECT_RADIUS, OBJECT_RADIUS));
		so.pEntity->addComponent(new KCCircleCollider(so.pEntity, OBJECT_RADIUS));
		//so.pPhysicsBody->setActivity(false);

		KBodyDef bodyDef;
		KMatDef matDef;
		matDef.restitution = 0.65f;
		bodyDef.bodyType = BodyType::Dynamic_Body;
		bodyDef.position = so.pEntity->m_pTransform->getPosition() / m_pPhysicsWorld->getPPM();
		matDef.density = so.getDensity();
		so.pPhysicsBody = new KCBody(*so.pEntity, bounds, bodyDef, matDef);

		so.pEntity->addComponent(so.pPhysicsBody);
		m_spaceThings.push_back(so);

	}

	GET_APP()->getRenderer()->addDebugShape(&line);
	GET_APP()->getRenderer()->showDebugDrawables(true);
	line.setCol(Colour::Magenta);


	m_pBackground = GET_SCENE()->addEntityToScene();
	m_pBackground->addComponent(new KCSprite(m_pBackground, Vec2f(GET_APP()->getWindowSize())));


	return KInitStatus::Success;
}

void GameSetup::onEnterScene()
{
	auto PlanetA = ASSET().getTexture(L"planet_a");
	auto PlanetB = ASSET().getTexture(L"planet_b");

	for (auto& so : m_spaceThings)
	{
		auto pSprite = so.pEntity->getComponent<KCSprite>();
		if (so.bIsPlanet)
		{
			pSprite->setTexture(PlanetA);
		}
		else
		{
			pSprite->setTexture(PlanetB);
			so.pPhysicsBody->setActivity(false);
		}
		pSprite->setColour(so.col);
	}

	m_pBackgroundShader = ASSET().getShader(L"heatmap");
	m_pBackground->getComponent<KCRenderableBase>()->setShader(m_pBackgroundShader);
	m_pBackground->getComponent<KCRenderableBase>()->setRenderLayer(-1);

	m_defaultView = GET_APP()->getRenderWindow()->getView();
}

void GameSetup::tick()
{
	for (auto& so : m_spaceThings)
	{
		if (so.bIsPlanet)
			so.mass = PLANET_MASS;
		else
			so.mass = OBJECT_MASS;

		so.pEntity->getComponent<KCBody>()->setDensity(so.getDensity());
	}
	static int idx = PLANETS_COUNT;
	if (KInput::MouseJustPressed(KMouseButton::Right))
	{
		if (idx < m_spaceThings.size())
		{
			m_spaceThings[idx].pEntity->setActive(true);
			m_spaceThings[idx].pEntity->m_pTransform->setPosition(KInput::GetMouseWorldPosition());
			m_spaceThings[idx].pEntity->m_pTransform->setRotation(0.0f);
			const Vec2f Position(m_spaceThings[idx].pEntity->m_pTransform->getPosition());
			m_spaceThings[idx].pPhysicsBody->setPosition(Position);
			float randAngle = Radians(RandFloat(0, 360.0f));

			Vec2f dir = m_spaceThings[0].pEntity->m_pTransform->getPosition() - Position;
			float length = GetLength(dir);
			dir /= length; // normalise dir 
			length /= PPM; // convert to Metres
			const Vec2f tangential = RotateVector(dir, 90.0f);

			// V = ¬/( G * M / r^2);
			float vel = sqrtf((G * m_spaceThings[0].mass) / length);
			KPrintf(L"Orbital Velocity is %f m/s\n", vel);
			float accel = vel / (GET_APP()->getPhysicsDelta());
			float forceMangitude = m_spaceThings[idx].mass * accel;

			Vec2f force = tangential * forceMangitude;
			m_spaceThings[idx].pPhysicsBody->setLinearVelocity(vel * tangential);
			m_spaceThings[idx].pPhysicsBody->setActivity(true);
			m_spaceThings[idx].pPhysicsBody->setRotation(0.0f);
			++idx;
		}

	}
	static bool bTailObject = false;
	static bool bHasZoomed = false;
	static uint32 objIdx = 0;

	if (bTailObject)
	{
		if (objIdx + (unsigned)(PLANETS_COUNT) > m_spaceThings.size())
		{
			objIdx = 0;
		}

		if (m_spaceThings[(unsigned)(PLANETS_COUNT)+objIdx].pEntity->isActive())
		{
			auto& thing = m_spaceThings[(unsigned)(PLANETS_COUNT)+objIdx];
			bHasZoomed = true;
			constexpr float ZoomAmount = 1.3f;
			auto f = KInput::GetMouseScrollDelta();
			if (f > 0.0f)
			{
				zoomAt(Vec2f(thing.pEntity->m_pTransform->getPosition()), 1.0f / ZoomAmount);
			}
			else if (f < 0.0f)
			{
				zoomAt(Vec2f(thing.pEntity->m_pTransform->getPosition()), ZoomAmount);
			}
			sf::View v = GET_APP()->getRenderWindow()->getView();
			v.setCenter(thing.pEntity->m_pTransform->getPosition());
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

	for (auto& so : m_spaceThings)
	{
		if (so.bIsPlanet || !so.pEntity->isActive())
		{
			continue;
		}

		Vec2f dir = m_spaceThings[0].pEntity->m_pTransform->getPosition() - so.pEntity->m_pTransform->getPosition();
		float length = GetLength(dir);
		dir /= length; // normalise dir 
		length /= PPM; // convert to Metres
		const Vec2f tangential = RotateVector(dir, 90.0f);

		// V = ¬/( G * M / r^2);
		float vel = sqrtf((G * m_spaceThings[0].mass) / length);
		float accel = vel / (GET_APP()->getPhysicsDelta());

		so.pPhysicsBody->setLinearVelocity(vel * tangential);
	}
	for (int32 i = 0; i < m_spaceThings.size(); ++i)
	{
		for (int32 j = i + 1; j < m_spaceThings.size(); ++j)
		{

			m_spaceThings[i];
			m_spaceThings[j];

			if (!m_spaceThings[i].pEntity->isActive() || !m_spaceThings[j].pEntity->isActive())
			{
				continue;
			}

			if ((!m_spaceThings[i].bIsPlanet && !m_spaceThings[j].bIsPlanet) || (m_spaceThings[i].bIsPlanet && m_spaceThings[j].bIsPlanet))
			{
				continue;
			}

			std::pair <SpaceObject*, SpaceObject*> pair;
			pair.first = &m_spaceThings[i];
			pair.second = &m_spaceThings[j];

			if (m_spaceThings[j].bIsPlanet)
			{
				pair.first = &m_spaceThings[i];
				pair.second = &m_spaceThings[j];
			}
			else
			{
				pair.first = &m_spaceThings[j];
				pair.second = &m_spaceThings[i];
			}

			Vec2f dir = pair.second->pEntity->m_pTransform->getPosition() - pair.first->pEntity->m_pTransform->getPosition();

			float distance = GetLength(dir) / m_pPhysicsWorld->getPPM();

			const float FORCE = G * pair.first->mass * pair.second->mass / ((distance * distance));
			if (!pair.first->bIsPlanet)
			{
				m_pPath->addPathPoint(pair.first->pEntity->m_pTransform->getPosition(), pair.first->col);
				//pair.first->pPhysicsBody->applyForceToCentre(FORCE * Normalise(dir));
			}

			if (!pair.second->bIsPlanet)
			{
				m_pPath->addPathPoint(pair.second->pEntity->m_pTransform->getPosition(), pair.second->col);
				//pair.second->pPhysicsBody->applyForceToCentre(FORCE * Normalise(-dir));

			}
		}
	}
}

void GameSetup::cleanUp()
{
	m_spaceThings.clear();
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
	std::vector<Vec2f> planetPositions;
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
	m_pBackgroundShader->setUniform("colScale", colScale);
	
}

float GameSetup::SpaceObject::getDensity()
{
	const float pmm = GET_APP()->getPhysicsWorld().getPPM();
	// Density is mass per unit area in Box2D
	// Area of space objects is calculated as a circle 
	// and Pixels per metre ratio is factored in
	return mass / (Maths::PI * (radius * radius) / pmm);
}
