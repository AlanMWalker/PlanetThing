#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>

#include "PlayerLocomotive.h"

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

PlayerLocomotive::PlayerLocomotive(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus PlayerLocomotive::init()
{
	auto pGod = GET_SCENE()->findEntityByTag(KTEXT("God"));

	if (!pGod)
	{
		KPRINTF("Couldn't find God in PlayerLocomotive!\n");
		return KInitStatus::Nullptr;
	}

	m_pImgui = pGod->getComponent<imguicomp>();

	// Whilst working on the movement we'll handle
	// setting up graphics here
	// should be moved to a different component afterwards
	m_pSprite = new KCSprite(getEntity(), Vec2f(32, 32));
	getEntity()->addComponent(m_pSprite);

	GET_APP()->getRenderer()->addDebugShape(&m_shape);
	m_shape.setRadius(20.0f);
	m_shape.setFillColor(sf::Color::Red);
	getEntity()->getTransform()->setTranslation(35, 64);

	return KInitStatus::Success;
}

void PlayerLocomotive::onEnterScene()
{
	m_pSprite->setTexture(ASSET().getTexture(KTEXT("fighter")));
	m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
}

void PlayerLocomotive::tick()
{
	m_pImgui->update();
	m_pImgui->begin("Box2D Testing");
	ImGui::SliderFloat("Move Speed", &m_moveSpeed, 0, 1000);
	m_pImgui->end();

	Vec2f dir;
	if (KInput::Pressed(KKey::W))
	{
		m_pSprite->setTextureRect(Recti{ 32,0,32,32 });
		dir.y = -1.0f;
	}
	else if (KInput::Pressed(KKey::S))
	{
		m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
		dir.y = 1.0f;
	}
	if (KInput::Pressed(KKey::A))
	{
		m_pSprite->setTextureRect(Recti{ 64,0,32,32 });
		dir.x = -1.0f;
	}
	else if (KInput::Pressed(KKey::D))
	{
		m_pSprite->setTextureRect(Recti{ 96, 0,32,32 });
		dir.x = 1.0f;
	}

	dir = Normalise(dir);
	getEntity()->getTransform()->move(dir * m_moveSpeed * GET_APP()->getDeltaTime());
}