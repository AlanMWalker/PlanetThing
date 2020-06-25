#include <KApplication.h>
#include "CelestialBody.hpp"

using namespace Krawler; 

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
		break;

	case CelestialBody::BodyType::Moon:
		KPRINTF("Moon is not yet a supported type\n");
		break;
	
	case CelestialBody::BodyType::Planet:

		break;
	}
	return KInitStatus::Success;
}

void CelestialBody::onEnterScene()
{

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
}

CelestialBody::BodyType CelestialBody::getBodyType() const
{
	return m_bodyType;
}

void CelestialBody::applyForce(const Vec2f& v)
{
	m_pBody->applyForceToCentre(v);
}

void CelestialBody::setupPlanet()
{
}

void CelestialBody::setupObject()
{
}
