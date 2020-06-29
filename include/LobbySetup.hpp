#pragma once 

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>

#include "SockSmeller.hpp"
#include "DbgImgui.hpp"



class LobbySetup :
	public Krawler::KComponentBase
{
public:


	LobbySetup();
	~LobbySetup() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	void setNetworkNodeType(NetworkNodeType type);
	void setHostLobbySize(Krawler::uint32 size);
	void setMyLobbyPort(Krawler::uint16 port);
	void setHostLobbyDetails(const sf::IpAddress& ip, Krawler::uint16 port);

private:

	sf::IpAddress m_lobbyHostIp; // Host IP that a client will connect to
	Krawler::uint16 m_lobbyHostPort; //Port a client will connect onto a host

	Krawler::uint16 m_myLobbyPort; //Port a host will open connections on when hosting a lobby

	imguicomp* m_pImguiComp = nullptr;
	NetworkNodeType m_nodeType = NetworkNodeType::Client;
	Krawler::uint32 m_lobbySize;

	void handleClientEstablish(ServerClientMessage*);
	void handleClientDisconnect(ServerClientMessage*);
};