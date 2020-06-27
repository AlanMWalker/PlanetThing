#pragma once 
#include "BaseController.hpp"

class imguicomp;

class LocalPlayerController :
	public BaseController
{
public:

	LocalPlayerController(CelestialBody* pHost);
	~LocalPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	imguicomp* m_pImgui = nullptr;

};