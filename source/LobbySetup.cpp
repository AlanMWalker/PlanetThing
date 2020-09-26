#include <string>
#include <sstream>

#include "LobbySetup.hpp"
#include "Blackboard.hpp"
#include "ServerPackets.hpp"
#include "GameSetup.hpp"
#include "Invoker.hpp"

using namespace Krawler;

LobbySetup::LobbySetup(GameSetup& gs)
	: KComponentBase(GET_SCENE_NAMED(Blackboard::LOBBY_SCENE)->addEntityToScene()), m_lobbySize((unsigned)Blackboard::MIN_NETWORKED),
	m_lobbyHostPort(0), m_myLobbyPort(0), m_gameSetup(gs)
{
	getEntity()->addComponent(this);
}

KInitStatus LobbySetup::init()
{
	auto entity = getEntity();
	entity->addComponent(new Invoker(entity));
	m_pImguiComp = new imguicomp(entity);
	entity->setTag(L"God");
	entity->addComponent(m_pImguiComp);


	return KInitStatus::Success;
}

void LobbySetup::onEnterScene()
{
	KPrintf(L"My network type is %s \n", m_nodeType == NetworkNodeType::Client ? L"Client" : L"Host");
	SockSmeller::get().setDisplayName(m_displayName);
	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		KPrintf(L"Setting up host lobby with port of %hu\n", m_myLobbyPort);
		SockSmeller::get().setupAsHost(m_myLobbyPort, m_lobbySize);
		m_lobbyState = LobbyState::HostWaiting;
		break;

	case NetworkNodeType::Client:
	{
		SockSmeller::get().setupAsClient(m_lobbyHostIp, m_lobbyHostPort);
		auto s = std::bind(&LobbySetup::handleClientEstablish, this);

		SockSmeller::Subscriber establish = [this](ServerClientMessage* scm) { handleClientEstablish(scm); };
		SockSmeller::Subscriber disconnect = [this](ServerClientMessage* scm) { handleClientDisconnect(scm); };
		SockSmeller::Subscriber lnl = [this](ServerClientMessage* scm) { handleClientLobbyNameList(scm); };
		SockSmeller::Subscriber gen = [this](ServerClientMessage* scm) { handleClientGenLevel(scm); };


		SockSmeller::get().subscribeToMessageType(MessageType::Establish, establish);
		SockSmeller::get().subscribeToMessageType(MessageType::Disconnect, disconnect);
		SockSmeller::get().subscribeToMessageType(MessageType::LobbyNameList, lnl);
		SockSmeller::get().subscribeToMessageType(MessageType::GeneratedLevel, gen);

		m_lobbyState = LobbyState::ClientConnecting;
	}
	default:
		break;
	}
	SockSmeller::get().setLobbyState(m_lobbyState);
}

void LobbySetup::tick()
{
	SockSmeller::get().setLobbyState(m_lobbyState);
	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		tickHost();
		break;

	case NetworkNodeType::Client:
		tickClient();
	default:
		break;
	}

}

void LobbySetup::setNetworkNodeType(NetworkNodeType type)
{
	m_nodeType = type;
}

void LobbySetup::setHostLobbySize(uint32 size)
{
	m_lobbySize = size;
}

void LobbySetup::setMyLobbyPort(uint16 port)
{
	m_myLobbyPort = port;
}

void LobbySetup::setHostLobbyDetails(const sf::IpAddress& ip, uint16 port)
{
	m_lobbyHostIp = ip;
	m_lobbyHostPort = port;
}

void LobbySetup::setDisplayName(const std::wstring& displayName)
{
	m_displayName = displayName;
}

void LobbySetup::tickClient()
{
	if (SockSmeller::get().isClientConnectionEstablished() && m_lobbyState == LobbyState::ClientConnecting)
	{
		m_lobbyState = LobbyState::ClientConnected;
	}

	m_pImguiComp->update();
	m_pImguiComp->begin("Lobby");
	switch (m_lobbyState)
	{
	case LobbyState::ClientConnecting:
		ImGui::Text("Establishing connection..");
		break;

	case LobbyState::ClientConnected:
		ImGui::Text("Waiting for players to join..");
		ImGui::Separator();
		ImGui::Text(" -- Connected users --");
		ImGui::Text(&TO_ASTR(m_displayName)[0]);
		for (auto& name : m_lobbyNamesClient)
		{
			ImGui::Text(&name[0]);
		}
		break;

	case LobbyState::ClientDisconnected:
	{
		ImGui::Text("Disconnected..");
		bool bTryAgain = ImGui::Button("Try again?");
		if (bTryAgain)
		{
			SockSmeller::get().tearDown();
			SockSmeller::get().setupAsClient(m_lobbyHostIp, m_lobbyHostPort);
			m_lobbyState = LobbyState::ClientConnecting;
		}

	}
	case LobbyState::None:
	case LobbyState::HostWaiting:
	default:
		break;
	}
	m_pImguiComp->end();
}

void LobbySetup::tickHost()
{
	static std::list<std::wstring> namesList;
	if (SockSmeller::get().hasNameListChanged())
	{
		namesList = SockSmeller::get().getConnectedUserDisplayNames();
	}

	bool bStartLobby = false;
	m_pImguiComp->update();
	m_pImguiComp->begin("Lobby");
	ImGui::Text("Waiting for players to join..");
	ImGui::Separator();
	ImGui::Text(" -- Connected users --");
	ImGui::Text(&TO_ASTR(m_displayName)[0]);
	for (auto& n : namesList)
	{
		ImGui::Text(&TO_ASTR(n)[0]);
	}

	bool bCancel = ImGui::Button("Cancel");
	if (bCancel)
	{
		m_lobbyState = LobbyState::HostCancel;
		SockSmeller::get().setLobbyState(m_lobbyState);
		SockSmeller::get().tearDown();
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::MENU_SCENE);
	}

	if (namesList.size() == (uint64)m_lobbySize)
	{
		bStartLobby = ImGui::Button("Start");
		if (bStartLobby)
		{
			m_gameSetup.setGameType(GameSetup::GameType::Networked);

		}
	}
	m_pImguiComp->end();

	if (bStartLobby)
	{
		GET_APP()->getSceneDirector().transitionToScene(Blackboard::GAME_SCENE);
	}
}

void LobbySetup::handleClientEstablish(ServerClientMessage* scm)
{
	KCHECK(scm);

	KPRINTF("Establish was replied to \n");
}

void LobbySetup::handleClientDisconnect(ServerClientMessage* scm)
{
	KCHECK(scm);
	KPRINTF("Client was disconnected by host \n");
	m_lobbyState = LobbyState::ClientDisconnected;
}

void LobbySetup::handleClientLobbyNameList(ServerClientMessage* scm)
{
	KCHECK(scm);
	if (!scm)
	{
		return;
	}

	m_lobbyNamesClient.clear();

	LobbyNameList& lnl = *((LobbyNameList*)(scm));
	std::stringstream ss(lnl.nameList);
	std::string name;
	while (std::getline(ss, name, ','))
	{
		m_lobbyNamesClient.push_back(name);
	}
}

void LobbySetup::handleClientGenLevel(ServerClientMessage* scm)
{
	GeneratedLevel& gen = *((GeneratedLevel*)(scm));
	m_gameSetup.setLevelGen(gen);
	m_lobbyState = LobbyState::InGame;
	GET_APP()->getSceneDirector().transitionToScene(Blackboard::GAME_SCENE);
}
