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
	return KInitStatus::Success;
}

void LobbySetup::onEnterScene()
{
	KPrintf(L"My network type is %s \n", m_nodeType == NetworkNodeType::Client ? L"Client" : L"Host");

	switch (m_nodeType)
	{
	case NetworkNodeType::Host:
		break;

	case NetworkNodeType::Client:
	default:
		break;
	}
}

void LobbySetup::tick()
{
}

void LobbySetup::setNetworkNodeType(NetworkNodeType type)
{
	m_nodeType = type;
}

void LobbySetup::setHostLobbySize(uint32 size)
{
	m_lobbySize = size;
}
