// engine
#include <Krawler.h>
#include <KApplication.h>
#include <Utilities\KDebug.h>

#include "GameSetup.hpp"
#include "MenuSetup.hpp"
#include "LobbySetup.hpp"
#include "Blackboard.hpp"

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

	std::wifstream input;
	input.open(L"version.txt");
	if (input.fail())
	{
		initApp.windowTitle = KTEXT("Planet Thing");
	}
	else
	{
		std::wstring version; 
		version.resize(20);
		input.getline(&version[0], 20, '\n');
		initApp.windowTitle = L"Planet Thing - " + version;
	}
	StartupEngine(&initApp);

	auto app = Krawler::KApplication::getApp();
	app->getSceneDirector().addScene(new KScene(Blackboard::MenuScene, Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().addScene(new KScene(Blackboard::GameScene, Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().addScene(new KScene(Blackboard::LobbyScene, Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().setStartScene(Blackboard::MenuScene);

	// Create menu and game setup
	// (don't need to delete as they will
	// be destroyed by KRAWLER)
	GameSetup* g = new GameSetup;
	LobbySetup* ls = new LobbySetup;
	MenuSetup* m = new MenuSetup(*g, *ls);
	
	InitialiseSubmodules();
	RunApplication();
	ShutdownEngine();
	SockSmeller::get().tearDown();
	delete &SockSmeller::get();

	_CrtDumpMemoryLeaks();

	return 0;
}


