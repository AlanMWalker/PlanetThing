#pragma once
#include <Krawler.h>
#include <KComponent.h>

#include <vector>

class TurnTaker
	: public Krawler::KComponentBase
{
public:

	TurnTaker(Krawler::KEntity* pEntity);
	~TurnTaker() = default;

	void setUUIDList(const std::vector<std::wstring>& list);
	void notifyTurnTaken();

private:
	
	// Pairing = UUID (string) - Is Turn Active(bool)
	std::map<std::wstring, bool> m_lobbyPlayers;

};

