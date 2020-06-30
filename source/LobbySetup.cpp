#include "LobbySetup.hpp"
#include "Blackboard.hpp"
#include "ServerPackets.hpp"

#include <string>
#include <sstream>

using namespace Krawler;

LobbySetup::LobbySetup()
	: KComponentBase(GET_SCENE_NAMED(Blackboard::LobbyScene)->addEntityToScene()), m_lobbySize((unsigned)Blackboard::MIN_NETWORKED),
	m_lobbyHostPort(0), m_myLobbyPort(0)
{
	getEntity()->addComponent(this);
}

KInitStatus LobbySetup::init()
{
	auto entity = getEntity();
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

		SockSmeller::get().subscribeToMessageType(MessageType::Establish, establish);
		SockSmeller::get().subscribeToMessageType(MessageType::Disconnect, disconnect);
		SockSmeller::get().subscribeToMessageType(MessageType::LobbyNameList, lnl);
		m_lobbyState = LobbyState::ClientConnecting;
	}
	default:
		break;
	}
}

void LobbySetup::tick()
{
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
	m_pImguiComp->update();
	m_pImguiComp->begin("Lobby");
	ImGui::Text("Waiting for players to join..");
	ImGui::Separator();
	ImGui::Text(" -- Connected users --");
	ImGui::Text(&TO_ASTR(m_displayName)[0]);
	auto namesList = SockSmeller::get().getConnectedUserDisplayNames();
	for (auto& n : namesList)
	{
		ImGui::Text(&TO_ASTR(n)[0]);
	}
	m_pImguiComp->end();
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
