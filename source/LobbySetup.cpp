#include "LobbySetup.hpp"
#include "Blackboard.hpp"

#include <string>


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

	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		KPrintf(L"Setting up host lobby with port of %hu\n", m_myLobbyPort);
		SockSmeller::get().setupAsHost(m_myLobbyPort);
		break;

	case NetworkNodeType::Client:
	{
		SockSmeller::get().setupAsClient(m_lobbyHostIp, m_lobbyHostPort);
		auto s = std::bind(&LobbySetup::handleClientEstablish, this);

		SockSmeller::Subscriber establish = [this](ServerClientMessage* scm) { handleClientEstablish(scm); };
		SockSmeller::Subscriber disconnect = [this](ServerClientMessage* scm) { handleClientDisconnect(scm); };

		SockSmeller::get().subscribeToMessageType(MessageType::Establish, establish);
		SockSmeller::get().subscribeToMessageType(MessageType::Disconnect, disconnect);
	}
	default:
		break;
	}
}

void LobbySetup::tick()
{
	m_pImguiComp->update();
	m_pImguiComp->begin("Lobby");
	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		ImGui::Text("Waiting for players to join..");
		break;

	case NetworkNodeType::Client:
		ImGui::Text("Establishing connection..");
	default:
		break;
	}
	m_pImguiComp->end();
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

void LobbySetup::handleClientEstablish(ServerClientMessage* scm)
{
	KPRINTF("Establish was replied to \n");
}

void LobbySetup::handleClientDisconnect(ServerClientMessage* scm)
{
	KPRINTF("Client was disconnected by host \n");
}
