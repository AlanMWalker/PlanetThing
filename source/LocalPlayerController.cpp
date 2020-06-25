#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Input/KInput.h>

#include "LocalPlayerController.hpp"
#include "Blackboard.hpp"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

LocalPlayerController::LocalPlayerController(CelestialBody* pHost)
	: KComponentBase(GET_APP()->getCurrentScene()->addEntityToScene()), m_pHostPlanet(pHost)
{
	getEntity()->setTag(L"Player_Satellite");
	getEntity()->addComponent(this);
	KCHECK(m_pHostPlanet);
}

KInitStatus LocalPlayerController::init()
{
	getEntity()->addComponent(new KCSprite(getEntity(), Vec2f(24, 24)));
	getEntity()->m_pTransform->setOrigin(12, 12);

	return KInitStatus::Success;
}

void LocalPlayerController::onEnterScene()
{
	updatePos();
}

void LocalPlayerController::tick()
{
	if (KInput::Pressed(KKey::A))
	{
		theta += -20.0f * GET_APP()->getDeltaTime();
	}

	if (KInput::Pressed(KKey::D))
	{
		theta += 20.0f * GET_APP()->getDeltaTime();
	}

	updatePos();
}

void LocalPlayerController::updatePos()
{
	Vec2f centre = m_pHostPlanet->getCentre();
	float r = Blackboard::PLAYER_ORBIT_RADIUS + m_pHostPlanet->getRadius();
	centre.x += cosf(Maths::Radians(theta)) * r;
	centre.y += sinf(Maths::Radians(theta)) * r;

	getEntity()->m_pTransform->setPosition(centre);
}
