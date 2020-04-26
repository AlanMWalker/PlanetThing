#pragma once

#include <Krawler.h>
#include <Components\KCSprite.h>

#include "KComponent.h"
#include "DbgImgui.h"

class PlayerLocomotive 
	: public Krawler::KComponentBase
{
public:

	PlayerLocomotive(Krawler::KEntity* pEntity);
	~PlayerLocomotive() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	

private:

	imguicomp* m_pImgui = nullptr;
	Krawler::Components::KCSprite* m_pSprite = nullptr;

};
