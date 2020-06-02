#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <Renderer/KRenderer.h>
#include <CompLocomotive.h>
#include <tgmath.h>

#include "BlockedMap.h"

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

CompLocomotive::CompLocomotive(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus CompLocomotive::init()
{
	auto pGod = GET_SCENE()->findEntity(KTEXT("God"));

	if (!pGod)
	{
		KPRINTF("Couldn't find God entity in CompLocomotive!\n");
		return KInitStatus::Nullptr;
	}

	m_pImgui = pGod->getComponent<imguicomp>();

	// Whilst working on the movement we'll handle
	// setting up graphics here
	// should be moved to a different component afterwards
	m_pSprite = new KCSprite(getEntity(), SIZE);

	getEntity()->addComponent(m_pSprite);
	getEntity()->getTransform()->setPosition(Maths::RandFloat(200, 300), Maths::RandFloat(200, 300));
	getEntity()->getTransform()->setOrigin(SIZE * 0.5f);

	m_Shape.setRadius(m_detectionRadius);
	m_Shape.setOrigin(m_detectionRadius, m_detectionRadius);
	m_Shape.setFillColor(sf::Color::Transparent);
	m_Shape.setOutlineColor(Colour::Green);
	m_Shape.setOutlineThickness(1);

	GET_APP()->getRenderer()->addDebugShape(&m_Shape);

	return KInitStatus::Success;
}

void CompLocomotive::onEnterScene()
{
	m_pSprite->setTexture(ASSET().getTexture(KTEXT("fighter")));
	m_pSprite->setTextureRect(Recti{ 0, 0, 32, 32 });
	//m_pSprite->setColour(sf::Color(0, 255, 0, 255));
}

void CompLocomotive::tick()
{
	detectEnermy();
}

void CompLocomotive::detectEnermy()
{
	Vec2f dir;

	auto myPos = getEntity()->getTransform()->getPosition();
	KEntity* playerEntity = GET_SCENE()->findEntity(L"Player");
	assert(playerEntity);

	Vec2f playerPos = playerEntity->getTransform()->getPosition();
	auto distanceToPlayer = GetSquareLength(playerPos - myPos);

	m_Shape.setPosition(myPos);
	
	if (distanceToPlayer <= (m_detectionRadius * m_detectionRadius)
		&& distanceToPlayer > SIZE.x * 2.0f)
	{
		if (!m_bIsOnPath)
		{
			m_path.clear();
			m_path = BlockedMap::getInstance().getWalkablePath(getEntity()->getTransform()->getPosition(), playerPos);
			m_whereWasPlayerLast = playerPos;
			m_bIsOnPath = true;
			m_pathIdx = 0;
		}
		else
		{
			if (m_pathIdx < m_path.size())
			{
				dir = m_path[m_pathIdx] - myPos;
				if (GetSquareLength(dir) > m_distFromPoint)
				{
					dir = Normalise(dir);
					manageIntersections(dir, GET_APP()->getDeltaTime(), m_moveSpeed);
					getEntity()->getTransform()->move(dir * m_moveSpeed * GET_APP()->getDeltaTime());
					++m_framesToContinue;
				}
				else
				{
					++m_pathIdx;
				}
			}
			else
			{
				KPRINTF("Finished Path\n");
				m_bIsOnPath = false;
			}

			if (GetSquareLength(playerPos - m_whereWasPlayerLast) > m_playerDistanceDelta && m_framesToContinue > MAX_CONTINUE_FRAMES)
			{
				m_bIsOnPath = false;
				m_framesToContinue = 0;
			}
		}
	}
}



void CompLocomotive::manageIntersections(Vec2f& dir, float dt, float speed)
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
			/*	if (i == 0)
					m_rayA.setStartEnd(startPointsX[i], endPointsX[i]);
				else
					m_rayB.setStartEnd(startPointsX[i], endPointsX[i]);*/

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
			/*if (i == 0)
				m_rayC.setStartEnd(startPointsY[i], endPointsY[i]);
			else
				m_rayD.setStartEnd(startPointsY[i], endPointsY[i]);*/

			const bool result = GET_APP()->getOverlord().castRayInScene(startPointsY[i], endPointsY[i], L"Terrain", getEntity());
			if (result)
			{
				dir.y = 0.0f;
			}
		}
	}

}

