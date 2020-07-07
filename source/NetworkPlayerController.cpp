#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Input/KInput.h>
#include <AssetLoader/KAssetLoader.h>

#include "NetworkPlayerController.hpp"
#include "Blackboard.hpp"

using namespace Krawler;
using namespace Krawler::Components;

NetworkPlayerController::NetworkPlayerController(KEntity* pEntity, CelestialBody* pHost)
	: BaseController(pEntity, pHost, Blackboard::PLAYER_SATELLITE_DIMENSION)
{
	getEntity()->setTag(L"Networked_Player_Satellite");
}

KInitStatus NetworkPlayerController::init()
{
	KPRINTF("Setting up network controller");
	auto result = BaseController::init();

	if (result != KInitStatus::Success)
	{
		return result;
	}

	auto playerTex = ASSET().getTexture(L"player_ship");
	getEntity()->getComponent<KCSprite>()->setTexture(playerTex);

	return KInitStatus::Success;
}

void NetworkPlayerController::onEnterScene()
{
	auto playerTex = ASSET().getTexture(L"player_ship");
	KCHECK(playerTex);

	SockSmeller::Subscriber handleMove = [this](ServerClientMessage* scm) { handlePlayerMoveHost(scm); };
	SockSmeller::Subscriber posUpdate = [this](ServerClientMessage* scm) { handlePosUpdate((SatellitePositionUpdate*)scm); };

	getEntity()->getComponent<KCSprite>()->setTexture(playerTex);
	auto b = []() {};

	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host)
	{
		SockSmeller::get().subscribeToMessageType(MessageType::MoveSatellite, handleMove);
	}
	else if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
	{
		SockSmeller::get().subscribeToMessageType(MessageType::SatellitePositionUpdate, posUpdate);
	}
	m_orbitRadius = Blackboard::PLAYER_ORBIT_RADIUS;
	BaseController::onEnterScene();
}

void NetworkPlayerController::tick()
{
	BaseController::tick();
	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host)
	{
		if (!m_moveSatQueue.empty())
		{
			while (!m_moveSatQueue.empty())
			{
				{
					std::lock_guard<std::mutex> guard(m_networkMtx);
					auto dir = m_moveSatQueue.front();
					dir = Maths::Clamp(-1, 1, dir);
					m_theta += dir * Blackboard::PLAYER_ENTITY_ROTATION_SPEED * GET_APP()->getDeltaTime();
					m_moveSatQueue.pop_front(); 
				}
				SockSmeller::get().hostSendMoveSatellite(m_theta, m_playerUUID);

			}
		}
	
	}
	else if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
	{
		if (m_bHasNewPos)
		{
			std::lock_guard<std::mutex> guard(m_networkMtx);
			m_bHasNewPos = false;
			m_theta = m_newTheta;
		}
	}
}

void NetworkPlayerController::handlePlayerMoveHost(ServerClientMessage* scm)
{
	MoveSatellite* ms = (MoveSatellite*)(scm);

	KCHECK(ms);
	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host)
	{
		if (ms->uuid == TO_ASTR(m_playerUUID))
		{
			std::lock_guard<std::mutex> guard(m_networkMtx);
			m_moveSatQueue.push_back(ms->direction);
		}
	}
}

void NetworkPlayerController::handlePosUpdate(SatellitePositionUpdate* spu)
{
	KCHECK(spu);

	if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
	{
		if (spu->uuid == TO_ASTR(m_playerUUID))
		{
			std::lock_guard<std::mutex> guard(m_networkMtx);
			m_bHasNewPos = true;
			m_newTheta = spu->theta;
		}
	}
}

