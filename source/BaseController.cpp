#include <Krawler.h>
#include <KApplication.h>
#include <AssetLoader/KAssetLoader.h>
#include <functional>

#include <Components/KCBoxCollider.h>


#include "BaseController.hpp"

using namespace Krawler;
using namespace Krawler::Components;

BaseController::BaseController(KEntity* pEntity, CelestialBody* pHost, const Krawler::Vec2f& size)
	: KComponentBase(pEntity), m_pHostPlanet(pHost), m_dimension(size), m_uuid(GenerateUUID())
{
	KCHECK(m_pHostPlanet);
	setComponentTag(L"BaseController");
}

KInitStatus BaseController::init()
{
	getEntity()->addComponent(new KCSprite(getEntity(), m_dimension));
	getEntity()->m_pTransform->setOrigin(m_dimension * 0.5f);

	auto God = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(God);


	for (auto& t : m_targets)
	{
		t.pEntity = GET_SCENE_NAMED(Blackboard::GameScene)->addEntityToScene();
		KCHECK(t.pEntity);
		if (!t.pEntity)
		{
			KPrintf(L"Unable to allocate entity for targets within %s init \n", getComponentTag().c_str());
			return KInitStatus::Nullptr;
		}

		t.pEntity->addComponent(new KCSprite(t.pEntity, Blackboard::TARGET_SIZE));
		t.pEntity->m_pTransform->setOrigin(Blackboard::TARGET_SIZE * 0.5f);
		t.pEntity->addComponent(new KCBoxCollider(t.pEntity, Blackboard::TARGET_SIZE));

		t.pEntity->setTag(L"Target");
	}

	return KInitStatus::Success;
}

void BaseController::onEnterScene()
{
	updateTransform();
	constructProjectileList();

	constexpr float delta = 360.0f / (float)(Blackboard::TARGET_COUNT);
	int i = 0;
	Vec2f pos;

	auto tex = ASSET().getTexture(L"target");
	tex->setSmooth(true);
	tex->generateMipmap();

	for (auto& t : m_targets)
	{
		pos.x = m_pHostPlanet->getCentre().x + cosf(Maths::Radians(delta * (float)(i))) * m_pHostPlanet->getRadius();
		pos.y = m_pHostPlanet->getCentre().y + sinf(Maths::Radians(delta * (float)(i))) * m_pHostPlanet->getRadius();

		t.pEntity->m_pTransform->setPosition(pos);
		++i;

		t.pEntity->getComponent<KCSprite>()->setTexture(tex);
		KCColliderFilteringData data;
		data.collisionMask = 0x0010;
		data.collisionFilter = 0x0001;
		t.callback = [this, t](const KCollisionDetectionData& d)->void {targetCollisionCallback(t, d); };

		t.pEntity->getComponent<KCBoxCollider>()->setCollisionFilteringData(data);
		t.pEntity->getComponent<KCBoxCollider>()->subscribeCollisionCallback(&t.callback);
	}

}

void BaseController::tick()
{
	updateTransform();
}

void BaseController::setHostPlanet(CelestialBody* pHost)
{
	m_pHostPlanet = pHost;
}

void BaseController::fireProjectile()
{
	// if it's not my turn I can't fire
	if (!m_bIsTurnActive)
	{
		return;
	}

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
			cb.get().spawnAtPoint(getEntity()->m_pTransform->getPosition(), Normalise(dir) * strength, m_uuid);
			m_shotCooldown.restart();
			break;
		}
	}
}

void BaseController::setTargetsInactive()
{
	for (auto t : m_targets)
	{
		t.pEntity->setActive(false);
	}
}

void BaseController::targetCollisionCallback(const Target& t, const Krawler::KCollisionDetectionData& data)
{
	KPrintf(L"Target was hit by %s\n", &data.entity->getTag()[0]);
	auto celestial = data.entity->getComponent<CelestialBody>();
	if (!celestial)
	{
		// Unexpected collision, just ignore it
		return;
	}

	auto spawnedBy = celestial->getSpawnedBy();
	if (spawnedBy.size() > 0)
	{
		if (spawnedBy != m_uuid)
		{
			t.pEntity->setActive(false);
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
