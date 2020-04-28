#include "GameSetup.h"
// engine
#include <Krawler.h>
#include <KApplication.h>

#include <Components\KCTileMap.h>

#include "DbgImgui.h"
#include "PlayerLocomotive.h"
#include "CompLocomotive.h"
#include "Camera.h"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;

GameSetup::GameSetup()
{
	createGod();
	createMap();
	createBlockedMap();
	createPlayer();
	createAi();
}

void GameSetup::createGod()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(KTEXT("God"));
	entity->addComponent(new imguicomp(entity));
}

void GameSetup::createMap()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(KTEXT("Map"));
	entity->addComponent(new KCTileMapSplit(entity, L"test_level"));

}

void GameSetup::createBlockedMap()
{
	m_blockedMap.setup(L"test_level");
}

void GameSetup::createPlayer()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(KTEXT("Player"));
	entity->addComponent(new PlayerLocomotive(entity));
	entity->addComponent(new Camera(entity));

}

void GameSetup::createAi()
{
	auto compEntity = GET_SCENE()->addEntityToScene();
	compEntity->setTag(KTEXT("Comp"));
	compEntity->addComponent(new CompLocomotive(compEntity));
}


