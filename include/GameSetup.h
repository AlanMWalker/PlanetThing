#pragma once

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>


namespace sf
{
	class Text;
}

struct DbgLineDraw
	: public sf::Drawable
{
	DbgLineDraw()
	{

		arr.setPrimitiveType(sf::PrimitiveType::Lines);
		arr.resize(2);
		arr[0].position = Krawler::Vec2f();
		arr[1].position = Krawler::Vec2f();
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(arr);
	}

	void setStartEnd(const Krawler::Vec2f& start, const Krawler::Vec2f& end)
	{
		arr[0].position = start;
		arr[1].position = end;
	}

	void setCol(const Krawler::Colour& c)
	{
		arr[0].color = c;
		arr[1].color = c;
	}
	sf::VertexArray arr;

};

class GameSetup :
	public Krawler::KComponentBase
{
public:

	GameSetup(Krawler::KEntity* pEntity);
	~GameSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void fixedTick() override; 

	float OBJECT_MASS = 200.0f;
	float PLANET_MASS = 9.8e13;
	float G = 6.67e-11;

private:

	struct SpaceObject
	{
		float mass;
		float radius;
		Krawler::Components::KCBody* pPhysicsBody;
		Krawler::KEntity* pEntity;
		bool bIsPlanet;

		float getDensity();
	};

	float PPM = 10.0f;

	void createGod();

	const float PLANET_RADIUS = 128.0f;
	const float OBJECT_RADIUS = 25.0f;


	const int32 PLANETS_COUNT = 1;
	const int32 OBJECTS_COUNT = 8;

	DbgLineDraw line;

	std::vector<SpaceObject> m_spaceThings;

	Krawler::KEntity* m_pObject;

	Krawler::Physics::KPhysicsWorld2D* m_pPhysicsWorld;



};