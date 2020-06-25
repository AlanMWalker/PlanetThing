#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Input/KInput.h>
#include <AssetLoader/KAssetLoader.h>

#include "LocalPlayerController.hpp"
#include "Blackboard.hpp"
#include "DbgImgui.hpp"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

LocalPlayerController::LocalPlayerController(CelestialBody* pHost)
	: KComponentBase(GET_SCENE_NAMED(Blackboard::GameScene)->addEntityToScene()), m_pHostPlanet(pHost)
{
	getEntity()->setTag(L"Player_Satellite");
	getEntity()->addComponent(this);
	KCHECK(m_pHostPlanet);
}

KInitStatus LocalPlayerController::init()
{
	getEntity()->addComponent(new KCSprite(getEntity(), Vec2f(24, 24)));
	getEntity()->m_pTransform->setOrigin(12, 12);

	auto God = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(God);
	m_pImgui = God->getComponent<imguicomp>();

	return KInitStatus::Success;
}

void LocalPlayerController::onEnterScene()
{
	updatePos();
	constructProjectileList();
}

void LocalPlayerController::tick()
{
	if (KInput::Pressed(KKey::A))
	{
		m_theta += -Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
	}

	if (KInput::Pressed(KKey::D))
	{
		m_theta += Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
	}

	updatePos();

	m_pImgui->begin("Player Controls");
	ImGui::PushFont(m_pImgui->getImguiFont());
	ImGui::SliderFloat("Shot Strength (%)", &m_shotStrength, 0.0f, 100.0f);
	bool bResult = ImGui::Button("Fire Projectile");
	ImGui::PopFont();
	m_pImgui->end();

	if (bResult)
	{
		for (auto cb : m_celestialbodies)
		{
			if (!cb.get().isActive())
			{
				Vec2f dir;
				dir = getEntity()->m_pTransform->getPosition() - m_pHostPlanet->getCentre();
				const float strength = (m_shotStrength / 100.0f) * 200;
				cb.get().spawnAtPoint(getEntity()->m_pTransform->getPosition(), Normalise(dir) * strength);
				break;
			}
		}
	}

}

void LocalPlayerController::updatePos()
{
	const float r = Blackboard::PLAYER_ORBIT_RADIUS + m_pHostPlanet->getRadius();

	Vec2f centre = m_pHostPlanet->getCentre();
	centre.x += cosf(Maths::Radians(m_theta)) * r;
	centre.y += sinf(Maths::Radians(m_theta)) * r;

	getEntity()->m_pTransform->setPosition(centre);
}

void LocalPlayerController::constructProjectileList()
{
	auto v = GET_SCENE()->getAllocatedEntityList();
	for (auto c : v)
	{
		auto cb = c->getComponent<CelestialBody>();
		if (cb)
		{
			if (cb->getBodyType() == CelestialBody::BodyType::Satellite)
			{
				m_celestialbodies.push_back(*cb);
			}
		}
	}
}
