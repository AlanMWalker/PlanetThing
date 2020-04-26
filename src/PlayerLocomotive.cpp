#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include "PlayerLocomotive.h"

using namespace Krawler;
using namespace Krawler::Components;

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
	ImGui::Text("Test Imgui");
	m_pImgui->end();
}