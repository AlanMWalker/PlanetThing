#include "GodDebugComp.h"
#include <Input\KInput.h>
#include <KApplication.h>

using namespace Krawler;
using namespace Krawler::Input;

GodDebugComp::GodDebugComp(Krawler::KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus GodDebugComp::init()
{
	return KInitStatus::Success;
}

void GodDebugComp::onEnterScene()
{
	auto pRenderer = GET_APP()->getRenderer();
	auto list = GET_SCENE()->getAllocatedEntityList();

	for (auto item : list)
	{
		if (item->getEntityTag() == L"Terrain")
		{
			sf::RectangleShape r;
			r = sf::RectangleShape(Vec2f(32, 32));
			r.setFillColor(Colour(255, 0, 0, 100));
			r.setPosition(item->getTransform()->getPosition());
			m_colliderShapes.push_back(r);
		}
	}

	for (auto& r : m_colliderShapes)
	{
		pRenderer->addDebugShape(&r);
	}
}

void GodDebugComp::tick()
{
	if (KInput::JustPressed(KKey::Escape))
	{
		// close
		GET_APP()->closeApplication();
	}
}

