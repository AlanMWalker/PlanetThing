// stl
#include <array>

// engine
#include <Krawler.h>
#include <KApplication.h>
#include <Utilities\KDebug.h>

#include "GameSetup.h"

#ifdef _DEBUG
// CRT 
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace Krawler;

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


	/*int a = 0xFFFFFFFF;
	int b = 0xF0;

	if (!(a & b))
	{
		std::cout << "Not same" << std::endl;
	}*/


	auto app = KApplication::getApp();
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Main_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().addScene(new KScene(std::wstring(KTEXT("Test_Scene")), Rectf(0, 0, (70 * 32), (40 * 32))));
	app->getSceneDirector().setStartScene(KTEXT("Main_Scene"));

	GameSetup g{};

	InitialiseSubmodules();
	RunApplication();
	ShutdownEngine();
	_CrtDumpMemoryLeaks();
	return 0;
}

