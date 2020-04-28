#pragma once

#include <Krawler.h>
#include <Components\KCSprite.h>
#include <SFML\Graphics\CircleShape.hpp>
#include "KComponent.h"
#include "DbgImgui.h"

class CompLocomotive
	: public Krawler::KComponentBase
{
public:

	CompLocomotive(Krawler::KEntity* pEntity);
	~CompLocomotive() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;


private:
	
	sf::CircleShape m_Shape;

	imguicomp* m_pImgui = nullptr;
	Krawler::Components::KCSprite* m_pSprite = nullptr;

	float m_moveSpeed = 100.0f;
};
