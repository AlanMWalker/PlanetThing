#pragma once 

#include <mutex> 
#include <atomic>

#include "BaseController.hpp"
#include "SockSmeller.hpp"

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

	void handlePosUpdateClient(ServerClientMessage* scm);
	void handleFireActivated(ServerClientMessage* scm);
	
	imguicomp* m_pImgui = nullptr;

	bool m_bHasNewPos = false;
	bool m_bFire = false;
	
	float m_newTheta = 0.0f;
};