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
	: BaseController(GET_SCENE_NAMED(Blackboard::GameScene)->addEntityToScene(), pHost, Blackboard::PLAYER_SATELLITE_DIMENSION)
{
	getEntity()->setTag(L"Player_Satellite");
	getEntity()->addComponent(this);
}

KInitStatus LocalPlayerController::init()
{
	auto result = BaseController::init();

	if (result != KInitStatus::Success)
	{
		return result;
	}

	auto playerTex = ASSET().getTexture(L"player_ship");
	getEntity()->getComponent<KCSprite>()->setTexture(playerTex);
	return KInitStatus::Success;
}

void LocalPlayerController::onEnterScene()
{
	auto God = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(God);
	m_pImgui = God->getComponent<imguicomp>();

	auto playerTex = ASSET().getTexture(L"player_ship");
	getEntity()->getComponent<KCSprite>()->setTexture(playerTex);

	m_orbitRadius = Blackboard::PLAYER_ORBIT_RADIUS;
	BaseController::onEnterScene();
}

void LocalPlayerController::tick()
{
	BaseController::tick();

	if (KInput::Pressed(KKey::A))
	{
		m_theta += -Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
	}

	if (KInput::Pressed(KKey::D))
	{
		m_theta += Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
	}

	m_pImgui->begin("Player Controls");
	ImGui::PushFont(m_pImgui->getImguiFont());
	ImGui::SliderFloat("Shot Strength (%)", &m_shotStrength, 0.0f, 100.0f);
	bool bResult = ImGui::Button("Fire Projectile");
	ImGui::PopFont();
	m_pImgui->end();

	if (bResult)
	{
		fireProjectile();
	}

	if (isTurnActive())
	{
		KPRINTF("ITS MY TURN\n");
	}
}

