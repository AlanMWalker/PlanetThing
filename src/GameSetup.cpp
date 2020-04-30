#include <GameSetup.h>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <DbgImgui.h>

// components
#include <GodDebugComp.h>
#include <Components\KCTileMap.h>
#include <PlayerLocomotive.h>
#include <Camera.h>
#include <Spawner.h>


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
}

void GameSetup::createGod()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"God");
	entity->addComponent(new imguicomp(entity));
	entity->addComponent(new GodDebugComp(entity));
}

void GameSetup::createMap()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"Map");
	entity->addComponent(new KCTileMapSplit(entity, L"test_level"));
	entity->addComponent(new Spawner(entity, Vec2f(Maths::RandFloat(100, 200), Maths::RandFloat(100, 200))));
}

void GameSetup::createBlockedMap()
{
	m_blockedMap.setup(L"test_level");
}

void GameSetup::createPlayer()
{
	auto entity = GET_SCENE()->addEntityToScene();
	entity->setTag(L"Player");
	entity->addComponent(new PlayerLocomotive(entity));
	entity->addComponent(new Camera(entity));
}

