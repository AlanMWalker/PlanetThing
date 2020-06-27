#include <Krawler.h>
#include <KApplication.h>

#include "Blackboard.hpp"
#include "BaseController.hpp"

using namespace Krawler;
using namespace Krawler::Components;

BaseController::BaseController(KEntity* pEntity, CelestialBody* pHost, const Krawler::Vec2f& size)
	: KComponentBase(pEntity), m_pHostPlanet(pHost), m_dimension(size)
{
	KCHECK(m_pHostPlanet);
}

KInitStatus BaseController::init()
{
	getEntity()->addComponent(new KCSprite(getEntity(), m_dimension));
	getEntity()->m_pTransform->setOrigin(m_dimension * 0.5f);

	auto God = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(God);
	return KInitStatus::Success;
}

void BaseController::onEnterScene()
{
	updateTransform();
	constructProjectileList();
}

void BaseController::tick()
{
	updateTransform();
}

void BaseController::fireProjectile()
{
	if (m_bFirstShot)
	{
		m_shotCooldown.restart();
		m_bFirstShot = false;
	}
	else if (m_shotCooldown.getElapsedTime().asSeconds() < 1)
	{
		return;
	}

	for (auto cb : m_objects)
	{
		if (!cb.get().isActive())
		{
			Vec2f dir;
			dir = getEntity()->m_pTransform->getPosition() - m_pHostPlanet->getCentre();
			const float strength = (m_shotStrength / 100.0f) * Blackboard::BOX2D_CAP;
			cb.get().spawnAtPoint(getEntity()->m_pTransform->getPosition(), Normalise(dir) * strength);
			m_shotCooldown.restart();
			break;
		}
	}
}

void BaseController::updateTransform()
{
	const float r = m_orbitRadius + m_pHostPlanet->getRadius() + (m_dimension.x / 2.0f);

	Vec2f centre = m_pHostPlanet->getCentre();
	centre.x += cosf(Maths::Radians(m_theta)) * r;
	centre.y += sinf(Maths::Radians(m_theta)) * r;

	getEntity()->m_pTransform->setPosition(centre);


	Vec2f dir = m_pHostPlanet->getCentre() - getEntity()->m_pTransform->getPosition();
	float theta = m_theta;//Maths::Degrees(atan2f(dir.y, dir.x));
	getEntity()->m_pTransform->setRotation(theta);
}

void BaseController::constructProjectileList()
{
	auto v = GET_SCENE()->getAllocatedEntityList();
	for (auto c : v)
	{
		auto cb = c->getComponent<CelestialBody>();
		if (cb)
		{
			if (cb->getBodyType() == CelestialBody::BodyType::Satellite)
			{
				m_objects.push_back(*cb);
			}
		}
	}
}
