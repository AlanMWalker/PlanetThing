#pragma once

#include "DbgImgui.hpp"

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

	GodDebugComp(Krawler::KEntity* pEntity);
	~GodDebugComp() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void onExitScene() override;

private: 

	void handleImgui();

	imguicomp* m_pImgui = nullptr;

	float m_freeCameraMoveSpeed = 50.0f;

	bool m_bShowDodgeSettings = false;
	bool m_bShowTerrainColliders = false;
};