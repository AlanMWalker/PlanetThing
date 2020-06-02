#pragma once

#include <Krawler.h>
#include <Components\KCSprite.h>
#include <SFML\Graphics\CircleShape.hpp>
#include <KComponent.h>
#include <DbgImgui.h>

class CompLocomotive
	: public Krawler::KComponentBase
{
public:

	CompLocomotive(Krawler::KEntity* pEntity);
	~CompLocomotive() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	void detectEnermy();
	void manageIntersections(Krawler::Vec2f& dir, float dt, float speed);

	const Krawler::Vec2f SIZE = Krawler::Vec2f(24, 24);
	const Krawler::Vec2f m_colliderBounds = (SIZE * 0.5f);
	const int32 MAX_CONTINUE_FRAMES = 60;
	sf::CircleShape m_Shape;

	imguicomp* m_pImgui = nullptr;
	Krawler::Components::KCSprite* m_pSprite = nullptr;

	std::vector<Krawler::Vec2f> m_path;

	Krawler::Vec2f m_whereWasPlayerLast;

	float m_moveSpeed = 100.0f;
	float m_detectionRadius = 150.0f;
	float m_t = 0.0f;
	float m_distFromPoint = 10.0f;
	float m_playerDistanceDelta = 1.0f;
	bool m_bIsOnPath = false;


	int32 m_pathIdx = -1;
	int32 m_framesToContinue = 0;
};
