#include <PlayerLocomotive.h>
// engine
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <Maths\KMaths.hpp>

#include "PlayerRenderableComp.h"
#include "NetworkComms.h"

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

PlayerLocomotive::PlayerLocomotive(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus PlayerLocomotive::init()
{
	auto pGod = GET_SCENE()->findEntity(KTEXT("God"));
	if (!pGod)
	{
		KPRINTF("Couldn't find God entity in PlayerLocomotive!\n");
		return KInitStatus::Nullptr;
	}


	{
		// Whilst working on the movement we'll handle
		// setting up graphics here
		// should be moved to a different component afterwards
		//getEntity()->getTransform()->setPosition(200, 250);
		getEntity()->getTransform()->setPosition(Maths::RandFloat(32, 300),
			Maths::RandFloat(32, 300));
		getEntity()->getTransform()->setOrigin(PLAYER_HALF_SIZE);

		GET_APP()->getRenderer()->addDebugShape(&m_colliderDebugShape);
		m_colliderDebugShape = sf::RectangleShape(m_colliderBounds);
		m_colliderDebugShape.setOrigin(m_colliderBounds * 0.5f);
		m_colliderDebugShape.setFillColor(Colour(0, 0, 255, 100));
	}
	{
		// Temporary place to attatch collider
		auto collider = new KCBoxCollider(getEntity(), m_colliderBounds);
		getEntity()->addComponent(collider);
		collider->subscribeCollisionCallback(&m_callback);
	}

	// DEBUG LINE
	m_rayA.setCol(Colour::Magenta);
	m_rayB.setCol(Colour::Magenta);
	m_rayC.setCol(Colour::Magenta);
	m_rayD.setCol(Colour::Magenta);

	GET_APP()->getRenderer()->addDebugShape(&m_rayA);
	GET_APP()->getRenderer()->addDebugShape(&m_rayB);
	GET_APP()->getRenderer()->addDebugShape(&m_rayC);
	GET_APP()->getRenderer()->addDebugShape(&m_rayD);
	return KInitStatus::Success;
}

void PlayerLocomotive::onEnterScene()
{

}

void PlayerLocomotive::tick()
{
	Vec2f dir;
	const float dt = GET_APP()->getDeltaTime();

	if (!sf::Joystick::isConnected(0))
	{
		handleKeyboardInput(dir, dt);
	}
	else
	{
		handleJoystickInput(dir, dt);
	}

	dir = Normalise(dir);
	handleDodge(dir, dt);

	const float speed = m_isDodging ? m_moveSpeed * m_dodgeMultiplyer : m_moveSpeed;

	manageIntersections(dir, dt, speed);
  
	getEntity()->getTransform()->move(dir * speed * dt);
	m_colliderDebugShape.setPosition(getEntity()->getTransform()->getPosition());
	static sf::Clock c;

	if (NetworkComms::get().isSpawnedIn())
	{
		if (c.getElapsedTime().asMilliseconds() > 55)
		{
			NetworkComms::get().moveInWorld(getEntity()->getTransform()->getPosition());
			c.restart();
		}
	}
}

void PlayerLocomotive::handleKeyboardInput(Vec2f& dir, float dt)
{
	auto renderComp = getEntity()->getComponent<PlayerRenderableComp>();

	if (!m_isDodging)
	{
		if (KInput::Pressed(UP))
		{

			dir.y = -1.0f;
		}
		if (KInput::Pressed(DOWN))
		{

			dir.y = 1.0f;
		}
		if (KInput::Pressed(LEFT))
		{

			dir.x = -1.0f;
		}
		if (KInput::Pressed(RIGHT))
		{

			dir.x = 1.0f;
		}
		if (KInput::JustPressed(DODGE) && !m_isDodging && m_canDodge && m_hasReleasedDodge)
		{
			m_isDodging = true;
			m_lastDir = dir;
			m_hasReleasedDodge = false;
			m_canDodge = false;
			return;
		}
		if (KInput::JustReleased(DODGE))
		{
			m_hasReleasedDodge = true;
		}

		static bool bUpPressed = false;
		static bool bDownPressed = false;
		static bool bLeftPressed = false;
		static bool bRightPressed = false;

		if (KInput::JustPressed(UP))
		{
			if (!bUpPressed)
			{
				bUpPressed = true;
				renderComp->startPlayingWalkAnim(WalkDir::Up);
			}
		}

		if (KInput::JustReleased(UP))
		{
			bUpPressed = false;
			renderComp->stopPlayingWalkAnim();
		}


		if (KInput::JustPressed(DOWN))
		{
			if (!bDownPressed)
			{
				bDownPressed = true;
				renderComp->startPlayingWalkAnim(WalkDir::Down);
			}
		}

		if (KInput::JustReleased(DOWN))
		{
			bDownPressed = false;
			renderComp->stopPlayingWalkAnim();
		}



		if (KInput::JustPressed(LEFT))
		{
			if (!bLeftPressed)
			{
				bLeftPressed = true;
				renderComp->startPlayingWalkAnim(WalkDir::Left);
			}
		}

		if (KInput::JustReleased(LEFT))
		{
			bLeftPressed = false;
			renderComp->stopPlayingWalkAnim();
		}

		if (KInput::JustPressed(RIGHT))
		{
			if (!bRightPressed)
			{
				bRightPressed = true;
				renderComp->startPlayingWalkAnim(WalkDir::Right);
			}
		}

		if (KInput::JustReleased(RIGHT))
		{
			bRightPressed = false;
			renderComp->stopPlayingWalkAnim();
		}

	}
	else
	{
		dir = m_lastDir;
	}
}

void PlayerLocomotive::handleJoystickInput(Krawler::Vec2f& dir, float dt)
{
	Vec2f moveAxis = Vec2f(sf::Joystick::getAxisPosition(0, sf::Joystick::X) / 100, sf::Joystick::getAxisPosition(0, sf::Joystick::Y) / 100);

	float deadzone = 0.2f;

	if (!m_isDodging)
	{
		if (moveAxis.x >= 0.10f)
		{
			dir.x = moveAxis.x;
		}
		else if (moveAxis.x <= -0.10f)
		{
			dir.x = moveAxis.x;
		}

		if (moveAxis.y >= 0.10f)
		{
			dir.y = moveAxis.y;
		}
		else if (moveAxis.y <= -0.10f)
		{
			dir.y = moveAxis.y;
		}

		if (sf::Joystick::isButtonPressed(0, 0) && !m_isDodging && m_canDodge && m_hasReleasedDodge)
		{
			m_isDodging = true;
			m_lastDir = dir;
			m_hasReleasedDodge = false;
			m_canDodge = false;
			return;
		}

		if (!sf::Joystick::isButtonPressed(0, 0))
		{
			m_hasReleasedDodge = true;
		}

	}
	else
	{
		dir = m_lastDir;
	}
}

void PlayerLocomotive::handleDodge(const Vec2f& dir, float dt)
{
	if (m_isDodging)
	{
		m_dodgeTimer += dt;

		if (m_dodgeTimer >= m_dodgeTiming)
		{
			m_isDodging = false;
			m_dodgeTimer = 0.0f;
		}
	}
	else if (!m_isDodging && !m_canDodge)
	{
		m_dodgeCDTimer += dt;
		if (m_dodgeCDTimer >= m_dodgeCooldown)
		{
			m_dodgeCDTimer = 0.0f;
			m_canDodge = true;
		}
	}
}

void PlayerLocomotive::manageIntersections(Vec2f& dir, float dt, float speed)
{
	const Vec2f currentPos = getEntity()->getTransform()->getPosition();
	const Vec2f halfSize = m_colliderBounds * 0.5f;

	const float fdirx = fabs(dir.x);
	const float fdiry = fabs(dir.y);
	// if we cancelled the vertical then the fabs(dir.x) is not 1 but it is also greater than 0
	// and the vertical component of dir is 0.0
	if (fdirx != 1.0f && fdirx > 0.0f && dir.y == 0.0f)
	{
		dir.x /= fdirx;
	}
	// if we cancelled the horizontal then the fabs(dir.y) is not 1 but it is also greater than 0
	// and the horizontal component of dir is 0.0
	else if (fdiry != 1.0f && fdiry > 0.0f && dir.x == 0.0f)
	{
		dir.y /= fdiry;
	}

	Vec2f startPointsX[2], endPointsX[2];
	Vec2f startPointsY[2], endPointsY[2];


	if (dir.x != 0.0f)
	{
		if (dir.x < 0.0f)
		{ // LEFT
			// TOP LEFT POINT
			startPointsX[0] = currentPos - halfSize;

			// BOTTOM LEFT POINT
			startPointsX[1] = currentPos - Vec2f(halfSize.x, -halfSize.y);
		}
		else
		{ // RIGHT 
			// TOP RIGHT POINT
			startPointsX[0] = currentPos + Vec2f(halfSize.x, -halfSize.y);

			// BOTTOM RIGHT POINT
			startPointsX[1] = currentPos + halfSize;
		}
		Vec2f tempDir = Vec2f(dir.x, 0);
		endPointsX[0] = startPointsX[0] + tempDir * speed * dt;
		endPointsX[1] = startPointsX[1] + tempDir * speed * dt;
	}

	if (dir.y != 0.0f)
	{
		if (dir.y < 0.0f)
		{ // UP
			// TOP LEFT POINT
			startPointsY[0] = currentPos - halfSize;

			// TOP RIGHT POINT
			startPointsY[1] = currentPos + Vec2f(halfSize.x, -halfSize.y);

		}
		else
		{ // DOWN
			// BOTTOM LEFT
			startPointsY[0] = currentPos - Vec2f(halfSize.x, -halfSize.y);

			// BOTTOM RIGHT
			startPointsY[1] = currentPos + halfSize;
		}
		Vec2f tempDir = Vec2f(0, dir.y);
		endPointsY[0] = startPointsY[0] + tempDir * speed * dt;
		endPointsY[1] = startPointsY[1] + tempDir * speed * dt;
	}

	// raycast x
	for (int i = 0; i < 2; ++i)
	{
		if (endPointsX[i] - startPointsX[i] != Vec2f())
		{
			if (i == 0)
				m_rayA.setStartEnd(startPointsX[i], endPointsX[i]);
			else
				m_rayB.setStartEnd(startPointsX[i], endPointsX[i]);

			const bool result = GET_APP()->getOverlord().castRayInScene(startPointsX[i], endPointsX[i], L"Terrain", getEntity());
			if (result)
			{
				dir.x = 0.0f;
			}
		}
	}

	// raycast y
	for (int i = 0; i < 2; ++i)
	{
		if (endPointsY[i] - startPointsY[i] != Vec2f())
		{
			if (i == 0)
				m_rayC.setStartEnd(startPointsY[i], endPointsY[i]);
			else
				m_rayD.setStartEnd(startPointsY[i], endPointsY[i]);

			const bool result = GET_APP()->getOverlord().castRayInScene(startPointsY[i], endPointsY[i], L"Terrain", getEntity());
			if (result)
			{
				dir.y = 0.0f;
			}
		}
	}

}

void PlayerLocomotive::resolve(const KCollisionDetectionData& collData)
{
}
