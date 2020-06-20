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

	// No gravity pls 
	m_pPhysicsWorld = &GET_APP()->getPhysicsWorld();
	m_pPhysicsWorld->setGravity(Vec2f(0.0f, 0.0f));
	m_pPhysicsWorld->setPPM(PPM);

	for (int i = 0; i < PLANETS_COUNT; ++i)
	{
		SpaceObject so;
		so.bIsPlanet = true;
		so.mass = PLANET_MASS;
		so.radius = PLANET_RADIUS;
		so.pEntity = GET_SCENE()->addEntityToScene();
		so.pEntity->setTag(L"Planet_" + to_wstring(i + 1));


		const Vec2f bounds(2.0f * PLANET_RADIUS, 2.0f * PLANET_RADIUS);

		so.pEntity->addComponent(new KCSprite(so.pEntity, bounds));
		so.pEntity->getTransform()->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

		so.pEntity->getTransform()->setPosition(Vec2f(
			//RandFloat(100, 600),
			//RandFloat(100, 500)
			GET_APP()->getWindowSize()
		) * 0.5f);

		so.pEntity->addComponent(new KCCircleCollider(so.pEntity, PLANET_RADIUS));
		KBodyDef bodyDef;
		KMatDef matDef;

		bodyDef.bodyType = BodyType::Dynamic_Body;
		bodyDef.position = so.pEntity->getTransform()->getPosition() / m_pPhysicsWorld->getPPM();
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
		const Vec2f bounds(2.0f * OBJECT_RADIUS, 2.0f * OBJECT_RADIUS);

		so.pEntity->setActive(false);
		so.pEntity->addComponent(new KCSprite(so.pEntity, bounds));
		so.pEntity->getTransform()->setOrigin(Vec2f(OBJECT_RADIUS, OBJECT_RADIUS));
		so.pEntity->addComponent(new KCCircleCollider(so.pEntity, OBJECT_RADIUS));
		//so.pPhysicsBody->setActivity(false);

		KBodyDef bodyDef;
		KMatDef matDef;

		bodyDef.bodyType = BodyType::Dynamic_Body;
		bodyDef.position = so.pEntity->getTransform()->getPosition() / m_pPhysicsWorld->getPPM();
		matDef.density = so.getDensity();
		so.pPhysicsBody = new KCBody(*so.pEntity, bounds, bodyDef, matDef);

		so.pEntity->addComponent(so.pPhysicsBody);
		m_spaceThings.push_back(so);

	}

	GET_APP()->getRenderer()->addDebugShape(&line);
	GET_APP()->getRenderer()->showDebugDrawables(true);
	line.setCol(Colour::Magenta);

	return KInitStatus::Success;
}

void GameSetup::onEnterScene()
{
	auto PlanetA = ASSET().getTexture(L"planet_a");
	auto PlanetB = ASSET().getTexture(L"planet_b");

	for (auto& so : m_spaceThings)
	{
		if (so.bIsPlanet)
		{
			so.pEntity->getComponent<KCSprite>()->setTexture(PlanetA);
		}
		else
		{
			so.pEntity->getComponent<KCSprite>()->setTexture(PlanetB);
			so.pPhysicsBody->setActivity(false);
		}
	}
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
			m_spaceThings[idx].pEntity->getTransform()->setPosition(KInput::GetMouseWorldPosition());
			m_spaceThings[idx].pEntity->getTransform()->setRotation(0.0f);
			const Vec2f Position(m_spaceThings[idx].pEntity->getTransform()->getPosition());
			m_spaceThings[idx].pPhysicsBody->setPosition(Position);
			float randAngle = Radians(RandFloat(0, 360.0f));

			//m_spaceThings[idx].pPhysicsBody->setLinearVelocity(Vec2f(0.0f, 0.0f));
			//m_spaceThings[idx].pPhysicsBody->applyForceToCentre(1.0e23f * Vec2f(cosf(randAngle), sinf(randAngle)));
			//m_spaceThings[idx].pPhysicsBody->setAngularVelocity(0.0f);

			Vec2f dir = m_spaceThings[0].pEntity->getTransform()->getPosition() - Position;
			float length = GetLength(dir);
			dir /= length;
			const Vec2f tangential = RotateVector(dir, 90.0f);

			float vel = sqrtf(G * m_spaceThings[0].mass / length);
			float accel = vel / GET_APP()->getPhysicsDelta();
			float forceMangitude = m_spaceThings[idx].mass * accel;

			auto test = GetLength(dir);
			Vec2f force = tangential * forceMangitude;
			//m_spaceThings[idx].pPhysicsBody->setLinearVelocity(Vec2f(tangential) * m_spaceThings[0].mass * G);
			m_spaceThings[idx].pPhysicsBody->applyForceToCentre(force);
			//m_spaceThings[idx].pPhysicsBody->applyLinearImpulseToCenter(force * 1000.0f);
			m_spaceThings[idx].pPhysicsBody->setActivity(true);
			m_spaceThings[idx].pPhysicsBody->setRotation(0.0f);
			++idx;
		}
		/*if (!m_pObject->isEntityActive())
		{
			m_pObject->setActive(true);
			m_pObject->getTransform()->setPosition(KInput::GetMouseWorldPosition());
			m_pObject->getComponent<KCBody>()->setPosition(m_pObject->getTransform()->getPosition());
		}*/

	}

	//if (m_pObject->isEntityActive())
	//{


	//	Vec2f dir = m_pPlanet->getTransform()->getPosition() - m_pObject->getTransform()->getPosition();
	//	float distance = GetLength(dir) / m_pPhysicsWorld->getPPM();
	//	const float FORCE = G * PLANET_MASS * OBJECT_MASS / ((distance * distance));

	//	KPrintf(L"Acceleration of %f M/S^2 applied \n", FORCE / OBJECT_MASS);

	//	line.arr[0] = m_pObject->getTransform()->getPosition();
	//	line.arr[1] = m_pObject->getTransform()->getPosition() + Normalise(dir) * 25.0f;

	//	m_pObject->getComponent<KCBody>()->applyForceToCentre(FORCE * Normalise(dir));


	//}
}

void GameSetup::fixedTick()
{
	for (int32 i = 0; i < m_spaceThings.size(); ++i)
	{
		for (int32 j = i + 1; j < m_spaceThings.size(); ++j)
		{

			m_spaceThings[i];
			m_spaceThings[j];

			if (!m_spaceThings[i].pEntity->isEntityActive() || !m_spaceThings[j].pEntity->isEntityActive())
			{
				continue;
			}

		/*	if ((!m_spaceThings[i].bIsPlanet && !m_spaceThings[j].bIsPlanet) || (m_spaceThings[i].bIsPlanet && m_spaceThings[j].bIsPlanet))
			{
				continue;
			}*/

			std::pair <SpaceObject*, SpaceObject*> pair;
			pair.first = &m_spaceThings[i];
			pair.second = &m_spaceThings[j];

			/*if (m_spaceThings[j].bIsPlanet)
			{
				pair.first = &m_spaceThings[i];
				pair.second = &m_spaceThings[j];
			}
			else
			{
				pair.first = &m_spaceThings[j];
				pair.second = &m_spaceThings[i];
			}*/

			Vec2f dir = pair.second->pEntity->getTransform()->getPosition() - pair.first->pEntity->getTransform()->getPosition();

			float distance = GetLength(dir) / m_pPhysicsWorld->getPPM();

			const float FORCE = G * pair.first->mass * pair.second->mass / ((distance * distance));
			//KPrintf(L"Force = %f Newtons\n", FORCE);
			//line.arr[0] = m_pObject->getTransform()->getPosition();
			//line.arr[1] = m_pObject->getTransform()->getPosition() + Normalise(dir) * 25.0f;
			if (!pair.first->bIsPlanet)
				pair.first->pPhysicsBody->applyForceToCentre(FORCE * Normalise(dir));

			if (!pair.second->bIsPlanet)
				pair.second->pPhysicsBody->applyForceToCentre(FORCE * Normalise(-dir));
			//pair.first->pPhysicsBody->applyLinearImpulseToCenter((FORCE * Normalise(dir)) * GET_APP()->getPhysicsDelta());
			//pair.first->pPhysicsBody->applyLinearImpulseToCenter(1.0e10f * Vec2f(0, 1)*10.0f);


		}
	}
}

void GameSetup::createGod()
{
	auto entity = getEntity();
	entity->setTag(L"God");
	entity->addComponent(new imguicomp(entity));
	entity->addComponent(new GodDebugComp(entity));
}

float GameSetup::SpaceObject::getDensity()
{
	const float pmm = GET_APP()->getPhysicsWorld().getPPM();
	// Density is mass per unit area in Box2D
	// Area of space objects is calculated as a circle 
	// and Pixels per metre ratio is factored in
	return mass / (Maths::PI * (radius * radius) / pmm);
}
