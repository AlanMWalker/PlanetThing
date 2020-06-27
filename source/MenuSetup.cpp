#include <KApplication.h>
#include <string>

#include "MenuSetup.hpp"
#include "Blackboard.hpp"
#include "GameSetup.hpp"

using namespace Krawler;


MenuSetup::MenuSetup(GameSetup& gs)
	: KComponentBase(GET_SCENE_NAMED(Blackboard::MenuScene)->addEntityToScene()), m_gs(gs)
{
	getEntity()->addComponent(this);
}

KInitStatus MenuSetup::init()
{

	auto entity = getEntity();
	m_pImguiComp = new imguicomp(entity);
	entity->setTag(L"God");
	entity->addComponent(m_pImguiComp);

	return KInitStatus::Success;
}

void MenuSetup::tick()
{
	static bool bMultiplayer = false;
	static bool bPlaySinglePlayer = false;
	static std::string ip;
	static std::string port;
	//std::string f;

	bool bPlayPressed = false;
	static int aiCount = 1;

	ip.resize(16);
	port.resize(6);
	m_pImguiComp->update();
	m_pImguiComp->begin("Planet Thing -- Menu");
	ImGuiWindowFlags f;
	auto before = bPlaySinglePlayer;
	ImGui::Checkbox("Play Single Player", &bPlaySinglePlayer);

	ImGui::Checkbox("Join Multiplayer Game", &bMultiplayer);
	if (bMultiplayer)
	{
		if (before == bPlaySinglePlayer)
			bPlaySinglePlayer = false;
		ImGui::InputText("Host IP", &ip[0], ip.size());
		ImGui::InputText("Host Port", &port[0], port.size());
		bool bTryLogin = ImGui::Button("Connect");
		if (bTryLogin)
		{
			KPrintf(L"User trying to connect to %s %s \n", TO_WSTR(ip).c_str(), TO_WSTR(port).c_str());
		}
	}



	if (bPlaySinglePlayer)
	{
		bMultiplayer = false;
		ImGui::SliderInt("Number of AI", &aiCount, Blackboard::MIN_AI, Blackboard::MAX_AI);
		bPlayPressed = ImGui::Button("Play");
	}

	m_pImguiComp->end();

	if (bPlayPressed)
	{
		m_gs.setAIPlayerCount(aiCount);
		m_gs.setGameType(GameSetup::GameType::Local);
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::GameScene);
	}
}
