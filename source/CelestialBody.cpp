#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Components/KCCircleCollider.h>
#include <AssetLoader/KAssetLoader.h>

#include "CelestialBody.hpp"
#include "Blackboard.hpp"

using namespace Krawler;
using namespace Krawler::Components;
using namespace Blackboard;

CelestialBody::CelestialBody(Krawler::KEntity* pEntity, CelestialBody::BodyType bodyType)
	: KComponentBase(pEntity), m_bodyType(bodyType)
{

}

KInitStatus CelestialBody::init()
{
	switch (m_bodyType)
	{
	default:
	case CelestialBody::BodyType::Satellite:
		setupSatellite();
		break;

	case CelestialBody::BodyType::Moon:
		KPRINTF("Moon is not yet a supported type\n");
		break;

	case CelestialBody::BodyType::Planet:
		setupPlanet();
		break;
	}
	return KInitStatus::Success;
}

void CelestialBody::onEnterScene()
{
	auto PlanetA = ASSET().getTexture(L"planet_a");
	auto PlanetB = ASSET().getTexture(L"planet_b");

	auto pSprite = getEntity()->getComponent<KCSprite>();
	if (m_bodyType == CelestialBody::BodyType::Planet)
	{
		pSprite->setTexture(PlanetA);
	}
	else
	{
		pSprite->setTexture(PlanetB);
		m_pBody->setActivity(false);
	}

	//pSprite->setColour(so.col);
}

void CelestialBody::fixedTick()
{
	// Handle putting path on screen
	// every certain time period

}

float CelestialBody::getMass()
{
	return m_mass;
}

Vec2f CelestialBody::getCentre()
{
	return getEntity()->m_pTransform->getPosition();
}

bool CelestialBody::isActive()
{
	return getEntity()->isActive();
}

void CelestialBody::spawnAtPoint(const Vec2f& position)
{
	// Grab spawn code from GameSetup

	getEntity()->setActive(true);
	getEntity()->m_pTransform->setPosition(position);
	getEntity()->m_pTransform->setRotation(0.0f);

	m_pBody->setRotation(0.0f);
	m_pBody->setPosition(position);
	m_pBody->setLinearVelocity(Vec2f(0.0f, 0.0f));
	m_pBody->setActivity(true);

	//const Vec2f Position(getEntity()->m_pTransform->getPosition());
	//m_spaceThings[idx].pPhysicsBody->setPosition(Position);
	//float randAngle = Radians(RandFloat(0, 360.0f));

	//Vec2f dir = m_spaceThings[0].pEntity->m_pTransform->getPosition() - Position;
	//float length = GetLength(dir);
	//dir /= length; // normalise dir 
	//length /= PPM; // convert to Metres
	//const Vec2f tangential = RotateVector(dir, 90.0f);

	//// V = ¬/( G * M / r^2);
	//float vel = sqrtf((G * m_spaceThings[0].mass) / length);
	//KPrintf(L"Orbital Velocity is %f m/s\n", vel);
	//float accel = vel / (GET_APP()->getPhysicsDelta());
	//float forceMangitude = m_spaceThings[idx].mass * accel;

	//Vec2f force = tangential * forceMangitude;
	//m_spaceThings[idx].pPhysicsBody->setLinearVelocity(vel * tangential);
	//m_spaceThings[idx].pPhysicsBody->setActivity(true);
}

CelestialBody::BodyType CelestialBody::getBodyType() const
{
	return m_bodyType;
}

void CelestialBody::applyForce(const Vec2f& v)
{
	m_pBody->applyForceToCentre(v);
}

float CelestialBody::getDensity()
{
	const float pmm = GET_APP()->getPhysicsWorld().getPPM();
	// Density is mass per unit area in Box2D
	// Area of space objects is calculated as a circle 
	// and Pixels per metre ratio is factored in
	return m_mass / (Maths::PI * (m_radius * m_radius) / pmm);
}

Vec2f CelestialBody::getVelocityInPixels()
{
	return m_pBody->getLinearVelocity() * GET_APP()->getPhysicsWorld().getPPM();
}

Vec2f CelestialBody::getVelocityInMetres()
{
	return m_pBody->getLinearVelocity();
}

void CelestialBody::setupPlanet()
{
	m_mass = Maths::RandFloat(PLANET_MASS, PLANET_MASS * 2);
	m_radius = PLANET_RADIUS;
	//so.col = genColour();
	getEntity()->setTag(L"Planet");

	const Vec2f bounds(2.0f * PLANET_RADIUS, 2.0f * PLANET_RADIUS);


	getEntity()->addComponent(new KCSprite(getEntity(), bounds));
	getEntity()->m_pTransform->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

	getEntity()->m_pTransform->setPosition(Vec2f(
		Maths::RandFloat(100, 1700),
		Maths::RandFloat(100, 900)
	));
	const float pmm = GET_APP()->getPhysicsWorld().getPPM();

	getEntity()->addComponent(new KCCircleCollider(getEntity(), PLANET_RADIUS));
	KBodyDef bodyDef;
	KMatDef matDef;
	matDef.restitution = 0.56f;
	bodyDef.bodyType = Krawler::Components::BodyType::Dynamic_Body;
	bodyDef.position = getEntity()->m_pTransform->getPosition() / pmm;
	matDef.density = getDensity();
	m_pBody = new KCBody(*getEntity(), bounds, bodyDef, matDef);
	getEntity()->addComponent(m_pBody);
}

void CelestialBody::setupSatellite()
{
	m_mass = OBJECT_MASS;
	m_radius = OBJECT_RADIUS;

	getEntity()->setTag(L"Object");
	//so.col = genColour();
	const Vec2f bounds(2.0f * OBJECT_RADIUS, 2.0f * OBJECT_RADIUS);

	getEntity()->setActive(false);
	getEntity()->addComponent(new KCSprite(getEntity(), bounds));
	getEntity()->m_pTransform->setOrigin(Vec2f(OBJECT_RADIUS, OBJECT_RADIUS));
	getEntity()->addComponent(new KCCircleCollider(getEntity(), OBJECT_RADIUS));
	//so.pPhysicsBody->setActivity(false);
	const float pmm = GET_APP()->getPhysicsWorld().getPPM();

	KBodyDef bodyDef;
	KMatDef matDef;
	matDef.restitution = 0.65f;
	bodyDef.bodyType = Krawler::Components::BodyType::Dynamic_Body;
	bodyDef.position = getEntity()->m_pTransform->getPosition() / pmm;
	matDef.density = getDensity();
	m_pBody = new KCBody(*getEntity(), bounds, bodyDef, matDef);

	getEntity()->addComponent(m_pBody);
}
