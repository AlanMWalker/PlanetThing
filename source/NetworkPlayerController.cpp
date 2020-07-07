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

	getEntity()->getComponent<KCSprite>()->setTexture(playerTex);

	m_orbitRadius = Blackboard::PLAYER_ORBIT_RADIUS;
	BaseController::onEnterScene();
}

void NetworkPlayerController::tick()
{
	BaseController::tick(); 
}

