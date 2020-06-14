#pragma once

#include "DbgImgui.h"

#include <Krawler.h>
#include <Components\KCSprite.h>
#include <SFML\Graphics\CircleShape.hpp>
#include <KComponent.h>

#include <SFML\Graphics\RectangleShape.hpp>

class GameSetup; 
class ServerPoll;
class ClientPoll;

class GodDebugComp
	: public Krawler::KComponentBase
{
public: 

	GodDebugComp(Krawler::KEntity* pEntity, GameSetup* pSetup, ServerPoll* pServerPoll, ClientPoll* pClientPoll);
	~GodDebugComp() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void onExitScene() override;

private: 

	void handlePathClicks();
	void handleShowTileChildren();
	void setStyle(imguicomp ImGui);
	void networkImgui();
	void handleImgui();
	void handleServerDebugCamera();
	void showNonServerDebugs();
	void showServerDebugs();

	std::vector<sf::RectangleShape> m_terrainColliderShapes;
	std::vector<sf::RectangleShape> m_tileAndChildren;
	std::vector<sf::RectangleShape> m_drawnPath;

	GameSetup* m_pSetup = nullptr;
	ServerPoll* m_pServerPoll = nullptr;
	ClientPoll* m_pClientPoll = nullptr;
	imguicomp* m_pImgui = nullptr;
	ImFont* m_pImguiFont = nullptr;

	float m_freeCameraMoveSpeed = 50.0f;

	bool m_bShowDodgeSettings = false;
	bool m_bShowTerrainColliders = false;
};