#pragma once 

#include <KComponent.h>
#include <Components/KCPhysicsBody.h>

class CelestialBody :
	public Krawler::KComponentBase
{
public:

	CelestialBody(Krawler::KEntity* pEntity);
	~CelestialBody() = default;

	float getMass();


private:

	float m_mass;


};