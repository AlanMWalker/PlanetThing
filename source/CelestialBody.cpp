#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Components/KCCircleCollider.h>
#include <AssetLoader/KAssetLoader.h>

#include "CelestialBody.hpp"
#include "Blackboard.hpp"
#include "ProjectilePath.hpp" 

using namespace Krawler;
using namespace Krawler::Components;
using namespace Blackboard;

Colour genColour()
{
	const uint8 r = static_cast<int8>(roundf((Maths::RandFloat() * 127) + 127));
	const uint8 g = static_cast<int8>(roundf((Maths::RandFloat() * 127) + 127));
	const uint8 b = static_cast<int8>(roundf((Maths::RandFloat() * 127) + 127));

	return Colour(r, g, b, 255);
}

CelestialBody::CelestialBody(Krawler::KEntity* pEntity, CelestialBody::BodyType bodyType, ProjectilePath& projPath, CelestialBody* pHostPlanet)
	: KComponentBase(pEntity), m_bodyType(bodyType), m_projPath(projPath), m_pHostPlanet(pHostPlanet)
{
	m_callBack = [this](const Krawler::KCollisionDetectionData& data) -> void
	{
		BodyType type;
		if (data.entityA != getEntity())
		{
			type = data.entityA->getComponent<CelestialBody>()->getBodyType();

		}
		else
		{
			type = data.entityB->getComponent<CelestialBody>()->getBodyType();
		}

		if (type == BodyType::Moon || type == BodyType::Planet)
		{
			setInActive();
			m_explosion.play();
		}
		else
		{
			m_slightHit.play();
		}
	};
}

KInitStatus CelestialBody::init()
{
	m_colour = genColour();
	switch (m_bodyType)
	{
	default:
	case CelestialBody::BodyType::Satellite:
		setupSatellite();
		break;

	case CelestialBody::BodyType::Moon:
		setupMoon();
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

	if (m_bodyType == BodyType::Satellite)
	{
		getEntity()->getComponent<KCColliderBase>()->subscribeCollisionCallback(&m_callBack);
	}

	auto sound = ASSET().getSound(L"rock_collide");
	m_explosion.setBuffer(*sound);

	sound = ASSET().getSound(L"glancing_collide");
	m_slightHit.setBuffer(*sound);
}

void CelestialBody::tick()
{
	if (m_bodyType == BodyType::Satellite)
	{
		if (m_satelliteAliveClock.getElapsedTime().asSeconds() > SATELLITE_ALIVE_TIME)
		{
			setInActive();
		}
	}
}

void CelestialBody::fixedTick()
{
	// Handle putting path on screen
	// every certain time period

	if (m_bodyType != BodyType::Planet && m_bodyType != BodyType::Moon)
	{
		m_projPath.addPathPoint(getEntity()->m_pTransform->getPosition(), m_colour);
	}

}

float CelestialBody::getMass() const
{
	return m_mass;
}

float CelestialBody::getRadius() const
{
	return m_radius;
}

Vec2f CelestialBody::getCentre()
{
	return getEntity()->m_pTransform->getPosition();
}

bool CelestialBody::isActive()
{
	return getEntity()->isActive();
}

void CelestialBody::spawnAtPoint(const Vec2f& position, const Vec2f& velocity)
{
	// Grab spawn code from GameSetup
	getEntity()->setActive(true);
	getEntity()->m_pTransform->setPosition(position);
	getEntity()->m_pTransform->setRotation(0.0f);

	m_pBody->setRotation(0.0f);
	m_pBody->setPosition(position);
	m_pBody->setLinearVelocity(velocity);
	m_pBody->setActivity(true);

	m_satelliteAliveClock.restart();
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

void CelestialBody::setInActive()
{
	getEntity()->setActive(false);
	m_pBody->setActivity(false);
}

void CelestialBody::setPosition(const Vec2f& pos)
{
	getEntity()->m_pTransform->setPosition(pos);
	m_pBody->setPosition(pos);

	getEntity()->setActive(true);
	m_pBody->setActivity(true);
}

void CelestialBody::setupPlanet()
{
	m_mass = Maths::RandFloat(PLANET_MASS, PLANET_MASS * 2);
	m_radius = PLANET_RADIUS;

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
	m_mass = SATELLITE_MAS;
	m_radius = SATELLITE_RADIUS;

	getEntity()->setTag(L"Satellite");

	const Vec2f bounds(2.0f * SATELLITE_RADIUS, 2.0f * SATELLITE_RADIUS);

	getEntity()->setActive(false);
	getEntity()->addComponent(new KCSprite(getEntity(), bounds));
	getEntity()->m_pTransform->setOrigin(Vec2f(SATELLITE_RADIUS, SATELLITE_RADIUS));
	getEntity()->addComponent(new KCCircleCollider(getEntity(), SATELLITE_RADIUS));

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

void CelestialBody::setupMoon()
{
	m_mass = MOON_MASS;
	m_radius = MOON_RADIUS;

	getEntity()->setTag(L"Moon");

	const Vec2f bounds(2.0f * MOON_RADIUS, 2.0f * MOON_RADIUS);

	getEntity()->setActive(false);
	getEntity()->addComponent(new KCSprite(getEntity(), bounds));
	getEntity()->m_pTransform->setOrigin(Vec2f(MOON_RADIUS, MOON_RADIUS));
	getEntity()->addComponent(new KCCircleCollider(getEntity(), MOON_RADIUS));
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
