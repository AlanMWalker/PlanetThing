#pragma warning(disable : 4996)
// stl
#include <array>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Utilities\KDebug.h>

#include <Components\KCTileMap.h>
#include <Components\KCBoxCollider.h>
#include <Components\KCBody.h>

#include "PlayerLocomotive.h"

#ifdef _DEBUG
// CRT 
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "DbgImgui.h"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::Maths;


class Box2DComp : public KComponentBase
{
public:

	Box2DComp(KEntity* pEntity) :
		KComponentBase(pEntity)
	{

	}

	virtual KInitStatus init() override
	{
		auto assets = KAssetLoader::getAssetLoader();
		auto result = assets.getLevelMap(KTEXT("test_level"));

		if (!result)
		{
			KPRINTF("FAILED TO LOAD MAP\n");
		}

		KPRINTF("SUCCEEDED IN LOADING MAP\n");
		getEntity()->addComponent(new KCTileMapSplit(getEntity(), L"test_level"));
		return KInitStatus::Success;
	}

	virtual void onEnterScene() override
	{
	}

	virtual void tick() override
	{
		static bool bOpen = true;

	}

private:

};

#ifndef _CONSOLE
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
#else
int main(void)
#endif
{
	int32 i = sf::Texture::getMaximumSize();
	KApplicationInitialise initApp(false);
	initApp.gameFps = 60;
	initApp.physicsFps = 60;
	initApp.width = 1024;
	initApp.height = 768;
	initApp.windowStyle = KWindowStyle::Windowed_Fixed_Size;
	initApp.windowTitle = KTEXT("Project Mountain Lion - InDev");
	StartupEngine(&initApp);

	auto app = KApplication::getApp();
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Main_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Test_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().setStartScene(KTEXT("Main_Scene"));

	//TODO move entity creation code
	// create God entity
	{
		auto entity = GET_SCENE()->addEntityToScene();
		entity->setTag(KTEXT("God"));
		entity->addComponent(new imguicomp(entity));
	}

	// Create Map Entity
	{
		auto entity = GET_SCENE()->addEntityToScene();
		entity->setTag(KTEXT("Map"));
		entity->addComponent(new KCTileMapSplit(entity, L"test_level"));
	}


	// Create player entity
	{
		auto entity = GET_SCENE()->addEntityToScene();
		entity->setTag(KTEXT("Player"));
		entity->addComponent(new PlayerLocomotive(entity));
	}


	InitialiseSubmodules();
	RunApplication();
	ShutdownEngine();
	_CrtDumpMemoryLeaks();
	return 0;
}

