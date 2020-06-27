#pragma once 

#include "BaseController.hpp"

class imguicomp;

class CPUPlayerController :
	public BaseController
{
public:

	CPUPlayerController(CelestialBody* pHost);
	~CPUPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

};