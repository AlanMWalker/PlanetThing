#pragma once

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>

#include "DbgImgui.hpp"


class MenuSetup :
	public Krawler::KComponentBase
{
public:

	MenuSetup();
	~MenuSetup() = default;
	
	virtual Krawler::KInitStatus init() override;
	virtual void tick() override;

private:

	imguicomp* m_pImguiComp = nullptr;

};