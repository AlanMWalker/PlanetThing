#pragma once

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>

#include "ProjectilePath.hpp"

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
	struct SpaceObject
	{
		float mass;
		float radius;
		Krawler::Components::KCBody* pPhysicsBody;
		Krawler::KEntity* pEntity;
		bool bIsPlanet;
		Krawler::Colour col = Krawler::Colour::White;
		float getDensity();
	};

	GameSetup(Krawler::KEntity* pEntity);
	~GameSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void fixedTick() override; 
	virtual void cleanUp() override;

	float colScale = 10000;
	std::vector<SpaceObject>& getSpaceThings() { return m_spaceThings; }

private:


	float PPM = 10.0f;

	void createGod();
	void zoomAt(const Krawler::Vec2f& pos, float zoom);
	void setBackgroundShaderParams();
	std::vector<Krawler::KEntity*> m_entities;

	DbgLineDraw line;
	ProjectilePath* m_pPath = nullptr; 
	std::vector<SpaceObject> m_spaceThings;
	sf::View m_defaultView;

	Krawler::KEntity* m_pObject;
	Krawler::KEntity* m_pBackground;

	sf::Shader* m_pBackgroundShader = nullptr;

	Krawler::Physics::KPhysicsWorld2D* m_pPhysicsWorld = nullptr;
};