#pragma once 

#include <KComponent.h>
#include <Components/KCPhysicsBody.h>

class CelestialBody;

class NewtonianGravity
{
public:

	void addCelestialBody(CelestialBody& celestialBody);
	void applyForces();

private:

	std::vector<std::reference_wrapper<CelestialBody>> m_celestialBodies;

};