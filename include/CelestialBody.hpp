#pragma once 

#include <SFML/Audio.hpp>

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

	CelestialBody(Krawler::KEntity* pEntity, BodyType bodyType, ProjectilePath& projPath, CelestialBody* pHostPlanet = nullptr);
	~CelestialBody() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void fixedTick() override;

	float getMass() const;
	float getRadius() const;
	Krawler::Vec2f getCentre();
	bool isActive();
	BodyType getBodyType() const;

	void spawnAtPoint(const Krawler::Vec2f& position, const Krawler::Vec2f& velocity = Krawler::Vec2f(), const std::wstring& spawnedByUUID = L"");
	void applyForce(const Krawler::Vec2f& v);

	float getDensity();

	Krawler::Vec2f getVelocityInPixels();
	Krawler::Vec2f getVelocityInMetres();

	void setInActive();

	CelestialBody* getHostPlanet() { return m_pHostPlanet; }

	float getOrbitTheta() const { return m_orbitTheta; }
	void setOrbitTheta(float theta) { m_orbitTheta = theta; }

	void setPosition(const Krawler::Vec2f& pos);

	// Can be empty if the entity spawning
	// did not specify its ownership of
	// the objects lifetime
	std::wstring getSpawnedBy() const { return m_spawnedByUUID; }

private:

	void setupPlanet();
	void setupSatellite();
	void setupMoon();


	void satelliteCallback(const Krawler::KCollisionDetectionData& data);
	const BodyType m_bodyType;

	Krawler::Components::KCColliderBaseCallback m_callBack;

	float m_mass = 0.0f;
	float m_radius = 0.0f;
	float m_orbitTheta = 0.0f; // only for moons

	std::wstring m_spawnedByUUID;

	Krawler::Components::KCBody* m_pBody = nullptr;
	CelestialBody* m_pHostPlanet;
	ProjectilePath& m_projPath;
	sf::Clock m_satelliteAliveClock;
	Krawler::Colour m_colour;

	sf::Sound m_explosion;
	sf::Sound m_slightHit;

};