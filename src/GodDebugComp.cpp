#include "GodDebugComp.h"
#include <Input\KInput.h>
#include <KApplication.h>

#include "PlayerLocomotive.h"

using namespace Krawler;
using namespace Krawler::Input;

GodDebugComp::GodDebugComp(Krawler::KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus GodDebugComp::init()
{
	m_pImgui = getEntity()->getComponent<imguicomp>();


	return KInitStatus::Success;
}

void GodDebugComp::onEnterScene()
{
	auto pRenderer = GET_APP()->getRenderer();
	auto list = GET_SCENE()->getAllocatedEntityList();

	for (auto item : list)
	{
		if (item->getTag() == L"Terrain")
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

	float* pMoveSpeed = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_moveSpeed;

	static bool bDebugShapes = GET_APP()->getRenderer()->isShowingDebugDrawables();

	m_pImgui->update();
	m_pImgui->begin("-- God Debug Tools --");
	ImGui::Checkbox("Show Debug Shapes", &bDebugShapes);
	ImGui::Separator();

	ImGui::Checkbox("Dodge Settings", &pShowDodgeSettings);
	if (pShowDodgeSettings)
	{
		auto pDodgeTiming = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeTiming;
		auto pDodgeCooldown = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeCooldown;
		auto pDodgeSpeed = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeMultiplyer;
		ImGui::SliderFloat("Dodge Timing", pDodgeTiming, 0.0f, 5.0f);
		ImGui::SliderFloat("Dodge Cooldown", pDodgeCooldown, 0.0f, 5.0f);
		ImGui::SliderFloat("Dodge Speed", pDodgeSpeed, 0.0f, 5.0f);
	}
	ImGui::InputFloat("Player Move Speed", pMoveSpeed);
	m_pImgui->end();

	GET_APP()->getRenderer()->showDebugDrawables(bDebugShapes);
}

