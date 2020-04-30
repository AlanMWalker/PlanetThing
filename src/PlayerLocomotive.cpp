#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>

#include "PlayerLocomotive.h"

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

PlayerLocomotive::PlayerLocomotive(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus PlayerLocomotive::init()
{
	auto pGod = GET_SCENE()->findEntityByTag(KTEXT("God"));
	if (!pGod)
	{
		KPRINTF("Couldn't find God in PlayerLocomotive!\n");
		return KInitStatus::Nullptr;
	}


	{// Whilst working on the movement we'll handle
	// setting up graphics here
	// should be moved to a different component afterwards
		m_pSprite = new KCSprite(getEntity(), Vec2f(32, 32));
		getEntity()->addComponent(m_pSprite);

		GET_APP()->getRenderer()->addDebugShape(&m_colliderDebugShape);
		m_colliderDebugShape = sf::RectangleShape(colliderBounds);
		m_colliderDebugShape.setOrigin(colliderBounds * 0.5f);
		m_colliderDebugShape.setFillColor(Colour(0, 0, 255, 100));
		getEntity()->getTransform()->setPosition(35, 64);
		getEntity()->getTransform()->setOrigin(16, 16);
	}
	{

		// Temporary place to attatch collider
		auto collider = new KCBoxCollider(getEntity(), colliderBounds);
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
	m_pSprite->setTexture(ASSET().getTexture(KTEXT("fighter")));
	m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
}

void PlayerLocomotive::tick()
{
	const float dt = GET_APP()->getDeltaTime();

	Vec2f dir;
	if (KInput::Pressed(KKey::W))
	{
		m_pSprite->setTextureRect(Recti{ 32,0,32,32 });
		dir.y = -1.0f;
	}
	if (KInput::Pressed(KKey::S))
	{
		m_pSprite->setTextureRect(Recti{ 0,0,32,32 });
		dir.y = 1.0f;
	}
	if (KInput::Pressed(KKey::A))
	{
		m_pSprite->setTextureRect(Recti{ 64,0,32,32 });
		dir.x = -1.0f;
	}
	if (KInput::Pressed(KKey::D))
	{
		m_pSprite->setTextureRect(Recti{ 96, 0,32,32 });
		dir.x = 1.0f;
	}

	dir = Normalise(dir);
	manageIntersections(dir, dt);
	getEntity()->getTransform()->move(dir * m_moveSpeed * dt);
	m_colliderDebugShape.setPosition(getEntity()->getTransform()->getPosition());

}

void PlayerLocomotive::manageIntersections(Vec2f& dir, float dt)
{
	// 
	const Vec2f currentPos = getEntity()->getTransform()->getPosition();
	const Vec2f halfSize = colliderBounds * 0.5f;

	Vec2f startPointsX[2], endPointsX[2];
	Vec2f startPointsY[2], endPointsY[2];

	if (dir.x != 0.0f)
	{
		if (dir.x == -1.0f)
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
		endPointsX[0] = startPointsX[0] + tempDir * m_moveSpeed * dt;
		endPointsX[1] = startPointsX[1] + tempDir * m_moveSpeed * dt;
	}


	if (dir.y != 0.0f)
	{
		if (dir.y == -1.0f)
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
		endPointsY[0] = startPointsY[0] + tempDir * m_moveSpeed * dt;
		endPointsY[1] = startPointsY[1] + tempDir * m_moveSpeed * dt;
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
				KPRINTF_A("Raycast hit for start point %s\n", dir.x > 0 ? L"Right" : L"Left");
				dir.x = 0.0f;
			}
		}
	}

	// raycast x
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
				KPRINTF_A("Raycast hit for start point %s\n", dir.y > 0 ? L"Down" : L"Up");
				dir.y = 0;
			}
		}
	}

}

void PlayerLocomotive::resolve(const KCollisionDetectionData& collData)
{
}
