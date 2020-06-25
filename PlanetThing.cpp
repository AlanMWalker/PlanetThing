//// stl
#include <array>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <Utilities\KDebug.h>

#include "GameSetup.hpp"

#ifdef _DEBUG
// CRT 
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;

using namespace Krawler;

#ifndef _DEBUG
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
#else
int main(void)
#endif
{
	int32 i = sf::Texture::getMaximumSize();
	KApplicationInitialise initApp(false);
	initApp.gameFps = 60;
	initApp.physicsFps = 120;
	initApp.width = 1920;//1024;
	initApp.height = 1080;//768;
	initApp.windowStyle = KWindowStyle::Windowed_Resizeable;
	initApp.windowTitle = KTEXT("Demo");
	StartupEngine(&initApp);

	auto app = KApplication::getApp();
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Main_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Menu_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().setStartScene(KTEXT("Main_Scene"));
	auto& entity = *app->getSceneDirector().getCurrentScene()->addEntityToScene();
	
	entity.addComponent(new GameSetup(&entity));

	InitialiseSubmodules();
	RunApplication();
	ShutdownEngine();
	_CrtDumpMemoryLeaks();
	return 0;
}


