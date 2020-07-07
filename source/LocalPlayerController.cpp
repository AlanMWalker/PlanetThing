#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Input/KInput.h>
#include <AssetLoader/KAssetLoader.h>

#include "LocalPlayerController.hpp"
#include "Blackboard.hpp"
#include "DbgImgui.hpp"
#include "GameSetup.hpp"

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
	
	auto god = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(god);
	auto type = god->getComponent<GameSetup>()->getGameType();
	if (type == GameSetup::GameType::Networked)
	{
		if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
		{
			SockSmeller::Subscriber posUpdate = [this](ServerClientMessage* scm) { handlePosUpdate((SatellitePositionUpdate*)scm); };
			SockSmeller::get().subscribeToMessageType(MessageType::SatellitePositionUpdate, posUpdate);
		}
	}
	BaseController::onEnterScene();
}

void LocalPlayerController::tick()
{
	BaseController::tick();

	auto god = GET_SCENE_NAMED(Blackboard::GameScene)->findEntity(L"God");
	KCHECK(god);

	if (!god)
	{
		return;
	}

	auto type = god->getComponent<GameSetup>()->getGameType();
	if (type == GameSetup::GameType::Local)
	{
		if (KInput::Pressed(KKey::A))
		{
			m_theta += -Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
		}

		if (KInput::Pressed(KKey::D))
		{
			m_theta += Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
		}
	}
	else // we must be networked
	{
		if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host)
		{
			auto before = m_theta;
			if (KInput::Pressed(KKey::A))
			{
				m_theta += -Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
			}

			if (KInput::Pressed(KKey::D))
			{
				m_theta += Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
			}

			// if we changed, then we'll move & send an update
			if (m_theta != before)
			{
				SockSmeller::get().hostSendMoveSatellite(m_theta, SockSmeller::get().getMyUUID());
			}
		}
		else
		{

			if (KInput::Pressed(KKey::A))
			{
				SockSmeller::get().clientSendSatelliteMove(-1);
			}

			if (KInput::Pressed(KKey::D))
			{
				SockSmeller::get().clientSendSatelliteMove(1);
			}
		}
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
	if (m_bHasNewPos)
	{
		std::lock_guard<std::mutex> guard(m_networkMtx);
		m_bHasNewPos = false;
		m_theta= m_newTheta;
	}
}

void LocalPlayerController::handlePosUpdate(ServerClientMessage* scm)
{
	auto spu = (SatellitePositionUpdate*)scm;
	KCHECK(spu);

	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
	{
		if (spu->uuid == TO_ASTR(SockSmeller::get().getMyUUID()))
		{
			std::lock_guard<std::mutex> guard(m_networkMtx);
			m_bHasNewPos = true;
			m_newTheta = spu->theta;
		}
	}
}

