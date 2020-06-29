#include <KApplication.h>
#include <string>

#include "MenuSetup.hpp"
#include "Blackboard.hpp"
#include "GameSetup.hpp"
#include "LobbySetup.hpp"

using namespace Krawler;


MenuSetup::MenuSetup(GameSetup& gs, LobbySetup& ls)
	: KComponentBase(GET_SCENE_NAMED(Blackboard::MenuScene)->addEntityToScene()), m_gs(gs), m_ls(ls)
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
	static bool bJoinMultiplayer = false;
	static bool bPlaySinglePlayer = false;
	static bool bHostMultiplayer = false;
	static std::string ip;
	//static std::string port;
	static int port;
	//std::string f;

	bool bPlayPressed = false;
	bool bHostPressed = false;
	bool bJoinPressed = false;

	static int aiCount = 1;
	static int playerLobbySize = 1;

	ip.resize(16);
	//port.resize(6);
	m_pImguiComp->update();
	m_pImguiComp->begin("Planet Thing -- Menu");
	ImGuiWindowFlags f;
	auto before = bPlaySinglePlayer;
	ImGui::Checkbox("Play Single Player", &bPlaySinglePlayer);

	ImGui::Checkbox("Join Multiplayer Game", &bJoinMultiplayer);

	ImGui::Checkbox("Host Multiplayer Game", &bHostMultiplayer);

	if (bJoinMultiplayer)
	{
		if (before == bPlaySinglePlayer)
			bPlaySinglePlayer = false;
		ImGui::InputText("Host IP", &ip[0], ip.size());
		ImGui::InputInt("Host Port (max port 65535)", &port);
		bJoinPressed = ImGui::Button("Connect");
	}

	if (bPlaySinglePlayer)
	{
		bJoinMultiplayer = false;
		bHostMultiplayer = false;
		ImGui::SliderInt("Number of AI", &aiCount, Blackboard::MIN_AI, Blackboard::MAX_AI);
		bPlayPressed = ImGui::Button("Play");
	}

	if (bHostMultiplayer)
	{
		ImGui::InputInt("Port to host on (max port 65535)", &port);
		ImGui::SliderInt("Number of players", &playerLobbySize, Blackboard::MIN_NETWORKED, Blackboard::MAX_NETWORKED);

		bHostPressed = ImGui::Button("Host Lobby");
	}

	m_pImguiComp->end();

	if (bPlayPressed)
	{
		m_gs.setAIPlayerCount(aiCount);
		m_gs.setGameType(GameSetup::GameType::Local);
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::GameScene);
		bJoinMultiplayer = false;
		bPlaySinglePlayer = false;
		bHostMultiplayer = false;
	}

	if (bJoinPressed)
	{
		m_ls.setHostLobbyDetails(ip, port);
		m_ls.setNetworkNodeType(NetworkNodeType::Client);
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::LobbyScene);
		bJoinMultiplayer = false;
		bPlaySinglePlayer = false;
		bHostMultiplayer = false;
	}
	
	if (bHostPressed)
	{
		m_ls.setHostLobbySize(playerLobbySize);
		m_ls.setMyLobbyPort((uint16)port);
		m_ls.setNetworkNodeType(NetworkNodeType::Host);
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::LobbyScene);
		bJoinMultiplayer = false;
		bPlaySinglePlayer = false;
		bHostMultiplayer = false;
	}

}
