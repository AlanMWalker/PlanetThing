#include "TurnTaker.hpp"

#include <KApplication.h>

#include "SockSmeller.hpp"
#include "Blackboard.hpp"
#include "LocalPlayerController.hpp"
#include "NetworkPlayerController.hpp"

using namespace Krawler;

TurnTaker::TurnTaker(KEntity* pEntity)
	: KComponentBase(pEntity)
{

}

void TurnTaker::onEnterScene()
{
	srand(5u);
	// Find all networked player entities
	auto pScene = GET_SCENE_NAMED(Blackboard::GAME_SCENE);
	auto list = pScene->getAllocatedEntityList();
	m_networkControllers.reserve(Blackboard::MAX_NETWORKED);

	for (auto entity : list)
	{
		auto controller = entity->getComponent<NetworkPlayerController>();
		if (!controller)
		{
			continue;
		}
		m_networkControllers.push_back(controller);
	}
	if (m_bIsNetworked)
	{
		if (SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Client)
			setupClientSubscribers();
	}
}

void TurnTaker::setUUIDList(const std::vector<std::wstring>& list)
{
	m_lobbyPlayers.clear();
	UserData ud;

	for (uint64 i = 0; i < list.size(); ++i)
	{
		auto uuid = list[i];
		if (i > 0)
		{
			ud.bIsTurnActive = false;
		}
		else
		{
			ud.bIsTurnActive = true;
		}

		ud.nextPlayer = i + 1 < list.size() ? list[i + 1] : list[0];

		m_lobbyPlayers.emplace(uuid, ud);
	}

	for (auto p : m_lobbyPlayers)
	{
		KPrintf(L"Found lobby player %s whose turn is %s and the next player is %s \n",
			p.first.c_str(),
			p.second.bIsTurnActive ? L"active" : L"inactive",
			p.second.nextPlayer.c_str()
		);
	}
}

void TurnTaker::notifyTurnTaken(const std::wstring& uuid)
{
	KCHECK(SockSmeller::get().getNetworkNodeType() == NetworkNodeType::Host); // Only hosts should run this code
	KCHECK(m_lobbyPlayers.size() > 1);// We need to have more than 1 player in our lobby to notify of turns

	if (m_lobbyPlayers.count(uuid) < 1)
		return;

	m_lobbyPlayers[uuid].bIsTurnActive = false;
	auto next = m_lobbyPlayers[uuid].nextPlayer;

	m_lobbyPlayers[next].bIsTurnActive = true;

	// If the host is next, enable the local player controller
	if (next == SockSmeller::get().getMyUUID())
	{
		auto pScene = GET_SCENE_NAMED(Blackboard::GAME_SCENE);
		auto pPlayer = pScene->findEntity(Blackboard::PLAYER_ENTITY_NAME);
		KCHECK(pPlayer);
		auto& lpc = *pPlayer->getComponent<LocalPlayerController>();
		lpc.setTurnIsActive(true);
	}
	else
	{
		// Otherwise, iterate over all networked players
		for (auto& c : m_networkControllers)
		{
			if (!c->getEntity()->isActive())
				continue;


			if (c->getUUID() == uuid)
			{
				c->setTurnIsActive(true);
			}
		}
	}

	SockSmeller::get().hostSendNextPlayerTurn(next);
}

void TurnTaker::setupClientSubscribers()
{
	SockSmeller::Subscriber npt = [this](ServerClientMessage* scm) {handleClientNextTurn(scm); };
	SockSmeller::get().subscribeToMessageType(MessageType::NextPlayerTurn, npt);
}

void TurnTaker::handleClientNextTurn(ServerClientMessage* scm)
{
	KCHECK(scm);
	if (!scm)
	{
		return;
	}

	NextPlayerTurn* npt = (NextPlayerTurn*)(scm);
	KCHECK(npt);

	if (!npt)
	{
		return;
	}

	if (TO_WSTR(npt->nextPlayerUUID) == SockSmeller::get().getMyUUID())
	{
		auto pScene = GET_SCENE_NAMED(Blackboard::GAME_SCENE);
		auto pPlayer = pScene->findEntity(Blackboard::PLAYER_ENTITY_NAME);
		KCHECK(pPlayer);
		auto& lpc = *pPlayer->getComponent<LocalPlayerController>();
		lpc.setTurnIsActive(true);
	}
	else
	{
		// Otherwise, iterate over all networked players
		for (auto& c : m_networkControllers)
		{
			if (c->getUUID() == TO_WSTR(npt->nextPlayerUUID))
			{
				c->setTurnIsActive(true);
			}
		}
	}
}
