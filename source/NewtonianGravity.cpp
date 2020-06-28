#include <utility>

#include <KApplication.h>

#include "NewtonianGravity.hpp"
#include "CelestialBody.hpp"
#include "Blackboard.hpp"

using namespace Krawler;
using namespace Blackboard;

NewtonianGravity::~NewtonianGravity()
{
	m_celestialBodies.clear();
}

void NewtonianGravity::addCelestialBody(CelestialBody& celestialBody)
{
	m_celestialBodies.push_back(celestialBody);
}

void NewtonianGravity::applyForces()
{
	auto& physWorld = GET_APP()->getPhysicsWorld();
	for (uint32 i = 0; i < m_celestialBodies.size(); ++i)
	{
		for (uint32 j = i + 1; j < m_celestialBodies.size(); ++j)
		{
			std::pair<CelestialBody&, CelestialBody&> pair(
				m_celestialBodies[i],
				m_celestialBodies[j]
			);

			if (!pair.first.isActive() || !pair.second.isActive())
			{
				continue;
			}

			if (pair.first.getBodyType() == CelestialBody::BodyType::Satellite &&
				pair.second.getBodyType() == CelestialBody::BodyType::Satellite)
			{
				continue;
			}

			if (pair.first.getBodyType() == CelestialBody::BodyType::Planet &&
				pair.second.getBodyType() == CelestialBody::BodyType::Planet)
			{
				continue;
			}


			Vec2f dir = pair.second.getCentre() - pair.first.getCentre();

			float distance = GetLength(dir) / physWorld.getPPM();

			const float FORCE = G * pair.first.getMass() * pair.second.getMass() / ((distance * distance));
			if (pair.first.getBodyType() != CelestialBody::BodyType::Planet &&
				pair.first.getBodyType() != CelestialBody::BodyType::Moon)
			{
				//m_pPath->addPathPoint(pair.first->pEntity->m_pTransform->getPosition(), pair.first->col);
				pair.first.applyForce(FORCE * Normalise(dir));
			}

			if (pair.second.getBodyType() != CelestialBody::BodyType::Planet &&
				pair.second.getBodyType() != CelestialBody::BodyType::Moon)
			{
				//m_pPath->addPathPoint(pair.second->pEntity->m_pTransform->getPosition(), pair.second->col);
				pair.second.applyForce(-FORCE * Normalise(dir));
			}

		}
	}

	float orbitSpeed = 50.0f;
	for (auto& b : m_celestialBodies)
	{
		if (!b.get().isActive())
		{
			continue;
		}

		if (b.get().getBodyType() == CelestialBody::BodyType::Moon)
		{
			auto host = b.get().getHostPlanet();
			float theta = b.get().getOrbitTheta();
			float deltaTheta = Maths::Radians(orbitSpeed) * GET_APP()->getDeltaTime();
			Vec2f centre = host->getCentre();

			float x = centre.x + (cosf(theta) * (MOON_RADIUS + PLANET_RADIUS + Blackboard::MOON_ORBIT_RADIUS));
			float y = centre.y + (sinf(theta) * (MOON_RADIUS + PLANET_RADIUS + Blackboard::MOON_ORBIT_RADIUS));
			theta += deltaTheta;
			b.get().setOrbitTheta(theta);
			b.get().setPosition(Vec2f(x, y));


		}
	}
}
