#include <Spawner.h>
// engine
#include <KApplication.h>
#include <Components/KCSprite.h>
#include <Renderer/KRenderer.h>
#include <AssetLoader\KAssetLoader.h>
#include <CompLocomotive.h>
#include <Input/KInput.h>

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::Input;

Spawner::Spawner(KEntity* pEntity, Vec2f spawnPoint, int numOfEntities)
	: KComponentBase(pEntity)
{
	this->spawnPoint = spawnPoint;
	this->numOfEntities = numOfEntities;
}

KInitStatus Spawner::init()
{
	m_pMap = GET_SCENE()->findEntityByTag(KTEXT("Map"));
	if (!m_pMap)
	{
		KPRINTF("Couldn't find Map entity on Spawner!\n");
		return KInitStatus::Nullptr;
	}

	m_entityList.resize(numOfEntities);
	for (auto &entity : m_entityList)
	{
		entity = GET_SCENE()->addEntityToScene();
		auto loco = new CompLocomotive(entity);
		entity->addComponent(loco);
		entity->getTransform()->setPosition(spawnPoint);
		entity->setActive(false);
	}

	return KInitStatus::Success;
}

void Spawner::onEnterScene()
{
}

void Spawner::tick()
{
	if (KInput::JustPressed(KKey::M)) { activateSpawner(); }
}

void Spawner::activateSpawner()
{
	for (auto &entity : m_entityList)
	{
		if (!entity->isEntityActive()) { entity->setActive(true); }
	}
}


