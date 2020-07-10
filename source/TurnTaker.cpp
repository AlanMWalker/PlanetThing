#include "TurnTaker.hpp"

using namespace Krawler;

TurnTaker::TurnTaker(KEntity* pEntity)
	: KComponentBase(pEntity)
{

}

void TurnTaker::setUUIDList(const std::vector<std::wstring>& list)
{
	m_lobbyPlayers.clear();
	for (auto& uuid : list)
	{
		if (uuid == *list.begin())
		{
			m_lobbyPlayers.emplace(uuid, true);
		}
		else
		{
			m_lobbyPlayers.emplace(uuid, false);
		}
	}

	for (auto p : m_lobbyPlayers)
	{
		KPrintf(L"Found lobby player %s whose turn is %s\n",
			p.first.c_str(),
			p.second ? L"active" : L"inactive"
		);
	}
}

void TurnTaker::notifyTurnTaken()
{
}
