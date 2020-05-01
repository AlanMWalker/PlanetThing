#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <Renderer/KRenderer.h>
#include <CompLocomotive.h>
#include <tgmath.h>

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

CompLocomotive::CompLocomotive(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus CompLocomotive::init()
{
	auto pGod = GET_SCENE()->findEntity(KTEXT("God"));

	if (!pGod)
	{
		KPRINTF("Couldn't find God entity in CompLocomotive!\n");
		return KInitStatus::Nullptr;
	}

	m_pImgui = pGod->getComponent<imguicomp>();

	// Whilst working on the movement we'll handle
	// setting up graphics here
	// should be moved to a different component afterwards
	m_pSprite = new KCSprite(getEntity(), Vec2f(32, 32));

	getEntity()->addComponent(m_pSprite);
	getEntity()->getTransform()->setPosition(Maths::RandFloat(200, 300), Maths::RandFloat(200, 300));
	getEntity()->getTransform()->setOrigin(16, 16);

	m_Shape.setRadius(100);
	m_Shape.setOrigin(100, 100);
	m_Shape.setFillColor(sf::Color::Transparent);
	m_Shape.setOutlineColor(Colour::Green);
	m_Shape.setOutlineThickness(1);
	GET_APP()->getRenderer()->addDebugShape(&m_Shape);

	return KInitStatus::Success;
}

void CompLocomotive::onEnterScene()
{
	m_pSprite->setTexture(ASSET().getTexture(KTEXT("fighter")));
	m_pSprite->setTextureRect(Recti{ 0, 0, 32, 32 });
	m_pSprite->setColour(sf::Color(0, 255, 0, 255));
}

void CompLocomotive::tick()
{
	detectEnermy();
}

void CompLocomotive::detectEnermy()
{
	Vec2f dir;

	auto playerEntity = GET_SCENE()->findEntity(L"Player");

	auto entityPos = playerEntity->getTransform()->getPosition();
	auto myPos = getEntity()->getTransform()->getPosition();

	auto distanceToPlayer = GetLength(entityPos - myPos);

	m_Shape.setPosition(myPos);

	dir = entityPos - myPos;

	if (distanceToPlayer <= 100)
	{
		dir = Normalise(dir);
		getEntity()->getTransform()->move(dir * m_moveSpeed * GET_APP()->getDeltaTime());
	}


}
