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

	void networkImgui();
	imguicomp* m_pImgui = nullptr;
	bool pShowDodgeSettings = false;

	std::vector<sf::RectangleShape> m_colliderShapes;
	std::vector<sf::RectangleShape> m_tileAndChildren;

	GameSetup* m_pSetup = nullptr;

	std::vector<sf::RectangleShape> m_drawnPath;

	void setStyle(imguicomp ImGui);

};