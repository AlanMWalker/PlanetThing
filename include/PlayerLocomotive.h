#pragma once

#include <Krawler.h>
#include <KComponent.h>
#include <SFML\Graphics\RectangleShape.hpp>
#include <Components\KCSprite.h>
#include <Components\KCBoxCollider.h>


#include <DbgImgui.h>

//Todo move this to its own class
struct DbgLine
	: public sf::Drawable
{
	DbgLine()
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

class PlayerLocomotive
	: public Krawler::KComponentBase
{
public:

	PlayerLocomotive(Krawler::KEntity* pEntity);
	~PlayerLocomotive() = default;

	float m_moveSpeed = 100.0f;
	
	float m_dodgeTiming = 0.4f;
	float m_dodgeCooldown = 1.0f;
	float m_dodgeMultiplyer = 2.0f;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	// KEYBOARD INPUT KEYS
	Krawler::Input::KKey UP = Krawler::Input::KKey::W;
	Krawler::Input::KKey DOWN = Krawler::Input::KKey::S;
	Krawler::Input::KKey LEFT = Krawler::Input::KKey::A;
	Krawler::Input::KKey RIGHT = Krawler::Input::KKey::D;
	Krawler::Input::KKey DODGE = Krawler::Input::KKey::Space;

	// JOY

	Krawler::Vec2f m_lastDir;

	bool m_isDodging = false;
	bool m_canDodge = true;
	bool m_hasReleasedDodge = true;
	float m_dodgeTimer = 0.0f;
	float m_dodgeCDTimer = 0.0f; 

	Krawler::KEntity* m_pGod = nullptr;
	sf::RectangleShape m_colliderDebugShape;

	const Krawler::Vec2f PLAYER_SIZE = Krawler::Vec2f(32, 32);
	Krawler::Vec2f m_colliderBounds = Krawler::Vec2f(28, 28);
	DbgLine m_rayA, m_rayB;
	DbgLine m_rayC, m_rayD;
	

	int m_frames = 0;


	void handleKeyboardInput(Krawler::Vec2f& dir, float dt);
	void handleJoystickInput(Krawler::Vec2f& dir, float dt);
	void handleDodge(const Krawler::Vec2f& dir, float dt);
	void manageIntersections(Krawler::Vec2f& dir, float dt);
	void resolve(const Krawler::KCollisionDetectionData& collData);

	Krawler::Components::KCColliderBaseCallback m_callback = [this](const Krawler::KCollisionDetectionData& collData)
	{
		resolve(collData);
	};

};
