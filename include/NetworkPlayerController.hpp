#pragma once 

#include "BaseController.hpp"


class NetworkPlayerController 
	: public BaseController
{
public:
	
	NetworkPlayerController(Krawler::KEntity* pEntity, CelestialBody* pHost);
	~NetworkPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

protected:
	
private:

	std::wstring m_playerUUID; 

};