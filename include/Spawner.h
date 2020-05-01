#pragma once

#include <Krawler.h>
#include <KApplication.h>
#include <KComponent.h>
#include <vector>

class Spawner
	: public Krawler::KComponentBase
{
public:
	Spawner(Krawler::KEntity* pEntity, Krawler::Vec2f spawnPoint, int numOfEntitys = 1);
	~Spawner() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

	enum class SpawnerType
	{
		Point = 0,
		Area = 1
	};

private:
	// Properties 
	Krawler::Vec2f m_spawnPoint;
	int m_numOfEntities;

	Krawler::KEntity* m_pMap = nullptr;
	std::vector<Krawler::KEntity*> m_entityList;
	Krawler::Components::KCSprite* m_pSprite = nullptr;

	void activateSpawner();
};