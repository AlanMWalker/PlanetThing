#include <Input\KInput.h>
#include <KApplication.h>

#include "GodDebugComp.hpp"
#include "GameSetup.hpp"

//#include "ServerPoll.hpp"
//#include "ClientPoll.hpp"

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


	ImGui::SFML::UpdateFontTexture();

}

void GodDebugComp::tick()
{
	// Just so game setup can handle spawning networked players on the 
	// correct thread we call a game setup tick since it's not 
	// a component, waking game setup up to check if it should make
	// a networked player visible
	handleImgui();
}

void GodDebugComp::onExitScene()
{
}

void GodDebugComp::handleImgui()
{
	if (KInput::JustPressed(KKey::Escape))
	{
		// close
		GET_APP()->closeApplication();
	}

	auto gameSetup = getEntity()->getComponent<GameSetup>();

	static bool bDebugShapes = GET_APP()->getRenderer()->isShowingDebugDrawables();
	static bool bShowTileNodes = false;

	m_pImgui->update();
	ImGui::PushFont(m_pImgui->getImguiFont());
	m_pImgui->begin("-- God Debug Tools --");
	ImGui::Separator();
	ImGui::SliderFloat("Gravity map colour multiplier", &gameSetup->colScale, 1.0f, 100000.0f);
	ImGui::PopFont();
	m_pImgui->end();

	GET_APP()->getRenderer()->showDebugDrawables(bDebugShapes);
}
