#pragma once 

#include <KComponent.h>

#include "CelestialBody.hpp"

class LocalPlayerController :
	public Krawler::KComponentBase
{
public:

	LocalPlayerController(CelestialBody* pHost);
	~LocalPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override; 
	virtual void tick() override; 

private: 
	void updatePos();

	float theta = 0.0f;
	CelestialBody* m_pHostPlanet = nullptr;
};