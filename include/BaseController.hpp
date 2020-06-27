#pragma once 

#include <KComponent.h>
#include "CelestialBody.hpp"


class imguicomp;

class BaseController :
	public Krawler::KComponentBase
{
public:

	BaseController(Krawler::KEntity* pEntity, CelestialBody* pHost, const Krawler::Vec2f& size);
	~BaseController() = default;


	// Any child classes should invoke the base init
	// at some stage during it's own implementation
	virtual Krawler::KInitStatus init() override;
	
	// Any child classes should invoke the base onEnterScene 
	// at some stage during it's own implementation
	virtual void onEnterScene() override;


	// Any child classes should invoke the base tick
	// at some stage during it's own implementation
	virtual void tick() override;

protected:

	void fireProjectile();
	
	CelestialBody* m_pHostPlanet = nullptr;


	float m_shotStrength = 50.0f;
	float m_theta = -90.0f;
	float m_orbitRadius = 1.0f;

private:

	void updateTransform();
	void constructProjectileList();
	
	sf::Clock m_shotCooldown;

	Krawler::Vec2f m_dimension;
	std::vector<std::reference_wrapper<CelestialBody>> m_objects;

	bool m_bFirstShot = true;
};