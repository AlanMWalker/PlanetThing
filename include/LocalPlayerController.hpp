#pragma once 

#include <KComponent.h>

#include "CelestialBody.hpp"


class imguicomp;

class LocalPlayerController :
	public Krawler::KComponentBase
{
public:

	LocalPlayerController(CelestialBody* pHost);
	~LocalPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

	float m_shotStrength = 50.0f;

private:
	void updatePos();
	void constructProjectileList();

	imguicomp* m_pImgui = nullptr;
	float m_theta = -90.0f;
	CelestialBody* m_pHostPlanet = nullptr;


	std::vector<std::reference_wrapper<CelestialBody>> m_celestialbodies;

};