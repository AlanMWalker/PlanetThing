#pragma once

#include <Krawler.h>
#include <KComponent.h>
#include <Components\KCSprite.h>
#include <Components\KCAnimatedSprite.h>
#include <AssetLoader/KAssetLoader.h>


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

	void startPlayingWalkAnim(WalkDir dir);
	void stopPlayingWalkAnim();

private:
	const Krawler::Vec2f PLAYER_SIZE = Krawler::Vec2f(128, 128);

	Krawler::Components::KCSprite* m_pSprite = nullptr;
	Krawler::Components::KCAnimatedSprite* m_pAnimatedSprite = nullptr;

	const std::wstring  LEFT_WALK_ANIM = L"player_left";
	const std::wstring  RIGHT_WALK_ANIM = L"player_right";
	const std::wstring  UP_WALK_ANIM = L"player_up";
	const std::wstring  DOWN_WALK_ANIM = L"player_down";
};