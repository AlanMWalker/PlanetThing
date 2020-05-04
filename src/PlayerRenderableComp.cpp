#include "PlayerRenderableComp.h"
#include <AssetLoader/KAssetLoader.h>

using namespace Krawler;
using namespace Krawler::Components;

PlayerRenderableComp::PlayerRenderableComp(Krawler::KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus PlayerRenderableComp::init()
{
	m_pSprite = new KCSprite(getEntity(), Vec2f(32, 32));
	getEntity()->addComponent(m_pSprite);

	return KInitStatus::Success;
}

void PlayerRenderableComp::onEnterScene()
{
	m_pSprite->setTexture(ASSET().getTexture(KTEXT("fighter")));
	m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
}

void PlayerRenderableComp::tick()
{
}

void PlayerRenderableComp::setWalkFrame(WalkDir dir)
{
	switch (dir)
	{
	case WalkDir::Left:
		m_pSprite->setTextureRect(Recti{ 64,0,32,32 });
		break;
	case WalkDir::Right:
		m_pSprite->setTextureRect(Recti{ 96, 0,32,32 });
		break;
	case WalkDir::Up:
		m_pSprite->setTextureRect(Recti{ 32,0,32,32 });
		break;
	case WalkDir::Down:
	default:
		m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
		break;
	}
}
