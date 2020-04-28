#include <KApplication.h>
#include "..\include\Camera.h"

using namespace Krawler;


Camera::Camera(KEntity* pEntity)
	: KComponentBase(pEntity)
{

}

KInitStatus Camera::init()
{
	m_pPlayer = getEntity();

	if (!m_pPlayer)
	{
		KPRINTF("Couldn't find Player in Camera!\n");
		return KInitStatus::Nullptr;
	}

	return KInitStatus::Success;
}

void Camera::onEnterScene()
{

}

void Camera::tick()
{
	Vec2f playerPos(m_pPlayer->getTransform()->getPosition());
	Vec2f windowSize(GET_APP()->getWindowSize());

	GET_APP()->getRenderWindow()->setView(sf::View(playerPos, windowSize));

}
