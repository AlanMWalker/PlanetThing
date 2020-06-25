#include <KApplication.h>
#include <string>

#include "MenuSetup.h"
#include "Blackboard.hpp"

using namespace Krawler;


MenuSetup::MenuSetup()
	: KComponentBase(GET_SCENE_NAMED(Blackboard::MenuScene)->addEntityToScene())
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
	static std::string ip;
	static std::string port;
	//std::string f;

	ip.resize(16);
	port.resize(6);
	m_pImguiComp->update();
	m_pImguiComp->begin("Planet Thing -- Menu");
	ImGuiWindowFlags f;
	bool bPlaySinglePlayer = ImGui::Button("Play Single Player");

	ImGui::Checkbox("Join Multiplayer Game", &bMultiplayer);
	if (bMultiplayer)
	{
		ImGui::InputText("Host IP", &ip[0], ip.size());
		ImGui::InputText("Host Port", &port[0], port.size());
		bool bTryLogin = ImGui::Button("Connect");
		if (bTryLogin)
		{
			KPrintf(L"User trying to connect to %s %s \n", TO_WSTR(ip).c_str(), TO_WSTR(port).c_str());
		}
	}
	m_pImguiComp->end();


	if (bPlaySinglePlayer)
	{
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::GameScene);
	}
}
