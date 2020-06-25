#include <KApplication.h>
#include "Camera.hpp"

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
		KPRINTF("Couldn't find Player entity in Camera!\n");
		return KInitStatus::Nullptr;
	}

	return KInitStatus::Success;
}

void Camera::onEnterScene()
{

}

void Camera::tick()
{
	if (getEntity()->getTag() != L"God")
	{
		Vec2f playerPos(m_pPlayer->m_pTransform->getPosition());
		Vec2f windowSize(GET_APP()->getWindowSize());
		sf::View v(playerPos, windowSize);
		GET_APP()->getRenderWindow()->setView(v);
	}
}

void Camera::moveCamera(const Vec2f& delta)
{
	sf::View v = GET_APP()->getRenderWindow()->getView();
	v.move(delta);
	GET_APP()->getRenderWindow()->setView(v);
}
