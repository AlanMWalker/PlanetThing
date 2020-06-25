#pragma once 

#include <KComponent.h>
#include <Components/KCBody.h>

class ProjectilePath;

class CelestialBody :
	public Krawler::KComponentBase
{
public:
	enum class BodyType : int32
	{
		Satellite = 0,
		Moon = 1,  // Not yet supported
		Planet = 2
	};

	CelestialBody(Krawler::KEntity* pEntity, BodyType bodyType, ProjectilePath& projPath);
	~CelestialBody() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void fixedTick() override;

	float getMass() const;
	float getRadius() const;
	Krawler::Vec2f getCentre();
	bool isActive();
	BodyType getBodyType() const;

	void spawnAtPoint(const Krawler::Vec2f& position, const Krawler::Vec2f& velocity = Krawler::Vec2f());
	void applyForce(const Krawler::Vec2f& v);

	float getDensity();

	Krawler::Vec2f getVelocityInPixels();
	Krawler::Vec2f getVelocityInMetres();

private:

	void setupPlanet();
	void setupSatellite();
	//void setupMoon(); Not yet implemented

	const BodyType m_bodyType;
	float m_mass = 0.0f;
	float m_radius = 0.0f;

	Krawler::Components::KCBody* m_pBody = nullptr;
	ProjectilePath& m_projPath;
	sf::Clock c;
	Krawler::Colour m_colour;

};