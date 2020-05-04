#pragma once

#include <Krawler.h>
#include <KComponent.h>
#include <Components\KCSprite.h>

enum class WalkDir : Krawler::int8
{
	Left,
	Right,
	Up,
	Down
};

class PlayerRenderableComp
	: public Krawler::KComponentBase
{
public:

	PlayerRenderableComp(Krawler::KEntity* pEntity);
	~PlayerRenderableComp() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

	void setWalkFrame(WalkDir dir);

private:
	const Krawler::Vec2f PLAYER_SIZE = Krawler::Vec2f(32, 32);

	Krawler::Components::KCSprite* m_pSprite = nullptr;

};