#include <KApplication.h>
#include <AssetLoader/KAssetLoader.h>
#include <Components/KCSprite.h>

#include "CPUPlayerController.hpp"
#include "Blackboard.hpp"

using namespace Krawler;
using namespace Krawler::Components;

CPUPlayerController::CPUPlayerController(CelestialBody* pHost)
	: BaseController(GET_SCENE_NAMED(Blackboard::GameScene)->addEntityToScene(), pHost, Blackboard::AI_SATELLITE_DIMENSION)
{
	getEntity()->setTag(L"AI_Satellite");
	getEntity()->addComponent(this);
}

KInitStatus CPUPlayerController::init()
{
	auto result = BaseController::init();
	if (result != KInitStatus::Success)
	{
		return result;
	}

	auto tex = ASSET().getTexture(L"enemy_ship");
	getEntity()->getComponent<KCSprite>()->setTexture(tex);

	return KInitStatus::Success;
}

void CPUPlayerController::onEnterScene()
{
	BaseController::onEnterScene();
	if (!m_pHostPlanet->isActive())
	{
		getEntity()->setActive(false);
		setTargetsInactive();
	}

	m_orbitRadius = Blackboard::PLAYER_ORBIT_RADIUS;
	m_theta = Maths::RandFloat(0.0, 359.0f);
}

void CPUPlayerController::tick()
{
	BaseController::tick();

	m_theta += 80.0f * GET_APP()->getDeltaTime();


	m_shotStrength = Maths::RandFloat(0.0f, 100.0f);
	//fireProjectile();

}

