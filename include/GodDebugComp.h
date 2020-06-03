#pragma once

#include "DbgImgui.h"

#include <Krawler.h>
#include <Components\KCSprite.h>
#include <SFML\Graphics\CircleShape.hpp>
#include <KComponent.h>

#include <SFML\Graphics\RectangleShape.hpp>
class GameSetup; 

class GodDebugComp
	: public Krawler::KComponentBase
{
public: 

	GodDebugComp(Krawler::KEntity* pEntity, GameSetup* pSetup);
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

	std::vector<sf::RectangleShape> m_terrainColliderShapes;
	std::vector<sf::RectangleShape> m_tileAndChildren;
	std::vector<sf::RectangleShape> m_drawnPath;

	GameSetup* m_pSetup = nullptr;
	imguicomp* m_pImgui = nullptr;
	ImFont* m_pImguiFont = nullptr;

	bool m_bShowDodgeSettings = false;
	bool m_bShowTerrainColliders = false;
};