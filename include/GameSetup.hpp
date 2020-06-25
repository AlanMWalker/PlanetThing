#pragma once

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>

#include "Blackboard.hpp"
#include "ProjectilePath.hpp"
#include "NewtonianGravity.hpp"
#include "LocalPlayerController.hpp"

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

class CelestialBody;

class GameSetup :
	public Krawler::KComponentBase
{
public:
	
	GameSetup();
	~GameSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void fixedTick() override; 
	virtual void cleanUp() override;

	float colScale = 10000;
	std::vector<Krawler::KEntity*>& getCelestialBodies() { return m_entities; }

	// Invoked by MenuSetup before it transitions into the singleplayer game
	void setAIPlayerCount(Krawler::int32 count);

private:


	float PPM = 10.0f;

	void createGod();
	void zoomAt(const Krawler::Vec2f& pos, float zoom);
	void setBackgroundShaderParams();
	std::vector<Krawler::KEntity*> m_entities;

	DbgLineDraw line;
	ProjectilePath* m_pPath = nullptr; 
	sf::View m_defaultView;

	Krawler::KEntity* m_pObject = nullptr;
	Krawler::KEntity* m_pBackground = nullptr;
	sf::Shader* m_pBackgroundShader = nullptr;
	Krawler::Physics::KPhysicsWorld2D* m_pPhysicsWorld = nullptr;
	LocalPlayerController* m_playerController = nullptr;
	
	NewtonianGravity m_newton;

	Krawler::int32 m_aiCount = Blackboard::MIN_AI;
};