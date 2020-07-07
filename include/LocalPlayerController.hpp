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

	void handlePosUpdate(ServerClientMessage* scm);
	
	imguicomp* m_pImgui = nullptr;
	std::mutex m_networkMtx;

	atombool m_bHasNewPos = false;
	float m_newTheta = 0.0f;
};