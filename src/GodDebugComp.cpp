#include "GodDebugComp.h"
#include <Input\KInput.h>
#include <KApplication.h>

#include "ClientPoll.h"
#include "PlayerLocomotive.h"
#include "GameSetup.h"
#include "ServerPoll.h"
#include "ClientPoll.h"
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

	// Sorry if there's any errors here, I translated this back by hand.
	auto& io = ImGui::GetIO();
	m_pImguiFont = io.Fonts->AddFontFromFileTTF("res/font//ChakraPetch-SemiBold.ttf", 15.0f);
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
	ImGui::PushFont(m_pImguiFont);
	m_pImgui->begin("-- God Debug Tools --");
	ImGui::SliderFloat("Planet Mass (KG)", &gameSetup->PLANET_MASS, 9.8e13 /2, 9.8e13 * 2);
	ImGui::SliderFloat("Object Mass (KG)", &gameSetup->OBJECT_MASS, 200 / 10.0f, 200 * 10.0f);
	double x = gameSetup->G;
	ImGui::InputDouble("Big G (Convoluted Units)", &x);//, 6.67e-11, 6.67e-9);
	gameSetup->G = x;
	//ImGui::Checkbox("Show Debug Shapes", &bDebugShapes);

	ImGui::Separator();

	ImGui::PopFont();
	m_pImgui->end();

	GET_APP()->getRenderer()->showDebugDrawables(bDebugShapes);
}
