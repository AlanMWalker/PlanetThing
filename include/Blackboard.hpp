
#pragma once
#include "Krawler.h"

namespace Blackboard
{
	// Scene Names

	static const wchar_t* MenuScene = L"Menu_Scene";
	static const wchar_t* GameScene = L"Main_Scene";

	const float G = 6.67e-11;

	// Masses
	constexpr float OBJECT_MASS = 200.0f;
	constexpr float PLANET_MASS = 9.8e13f;
	constexpr float MOON_MASS = PLANET_MASS / 100.0f;


	// Distances 
	constexpr float PLANET_RADIUS = 64.0f;
	constexpr float OBJECT_RADIUS = 8.0f;
	constexpr float PLAYER_ORBIT_RADIUS = 50.0f;

	// Populations 
	constexpr Krawler::int32 PLANETS_COUNT = 5;
	constexpr Krawler::int32 OBJECTS_COUNT = 20;

	constexpr float PLAYER_ENTITY_ROTATION_SPEED = 50.0f;

}