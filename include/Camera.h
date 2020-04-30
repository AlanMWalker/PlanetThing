#pragma once

#include <Krawler.h>
#include "KComponent.h"

class Camera
	: public Krawler::KComponentBase
{
public:

	Camera(Krawler::KEntity* pEntity);
	~Camera() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:
	Krawler::KEntity* m_pPlayer = nullptr;
};