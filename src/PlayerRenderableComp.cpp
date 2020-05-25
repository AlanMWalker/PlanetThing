#include "PlayerRenderableComp.h"

using namespace Krawler;
using namespace Krawler::Components;

PlayerRenderableComp::PlayerRenderableComp(Krawler::KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus PlayerRenderableComp::init()
{
	m_pAnimatedSprite = new KCAnimatedSprite(getEntity(), ASSET().getAnimation(L"player_left"));
	m_pAnimatedSprite->setRepeatingState(true);
	getEntity()->addComponent(m_pAnimatedSprite);

	return KInitStatus::Success;
}

void PlayerRenderableComp::onEnterScene()
{
	m_pAnimatedSprite->play();
}

void PlayerRenderableComp::tick()
{
}

void PlayerRenderableComp::startPlayingWalkAnim(WalkDir dir)
{
	m_pAnimatedSprite->stop();
	switch (dir)
	{
	case WalkDir::Left:
			m_pAnimatedSprite->setAnimation(LEFT_WALK_ANIM);
			m_pAnimatedSprite->play();
		break;
	case WalkDir::Right:
		m_pAnimatedSprite->setAnimation(RIGHT_WALK_ANIM);
		m_pAnimatedSprite->play();
		break;
	case WalkDir::Up:
		m_pAnimatedSprite->setAnimation(UP_WALK_ANIM);
		m_pAnimatedSprite->play();
		break;
	case WalkDir::Down:
	default:
		m_pAnimatedSprite->setAnimation(DOWN_WALK_ANIM);
		m_pAnimatedSprite->play();
		break;
	}
}

void PlayerRenderableComp::stopPlayingWalkAnim()
{
	m_pAnimatedSprite->stop();
}
