#pragma once
#include <Krawler.h>
#include <KComponent.h>

#include <vector>

class NetworkPlayerController;
struct ServerClientMessage;

class TurnTaker
	: public Krawler::KComponentBase
{
public:

	TurnTaker(Krawler::KEntity* pEntity);
	~TurnTaker() = default;

	virtual void onEnterScene() override; 


	void setUUIDList(const std::vector<std::wstring>& list);
	void notifyTurnTaken(const std::wstring& uuid);

	void setIsNetworked(bool bIsNetworked) { m_bIsNetworked = bIsNetworked; }
private:
	
	void setupClientSubscribers();

	void handleClientNextTurn(ServerClientMessage* scm);

	// Pairing = UUID (string) - Is Turn Active(bool)
	struct UserData
	{
		std::wstring nextPlayer; 
		bool bIsTurnActive = false;
	};

	std::map<std::wstring, UserData> m_lobbyPlayers;
	std::vector<NetworkPlayerController*> m_networkControllers;

	bool m_bIsNetworked = false;
};

