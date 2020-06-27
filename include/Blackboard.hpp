
#pragma once
#include "Krawler.h"

namespace Blackboard
{
	// Scene Names

	static const wchar_t* MenuScene = L"Menu_Scene";
	static const wchar_t* GameScene = L"Main_Scene";

	// Constants for physics
	const float G = 6.67e-11; // m^3 kg^–1 s^–2
	const float BOX2D_CAP = 200; //m/s

	// Masses
	constexpr float SATELLITE_MAS = 200.0f;
	constexpr float PLANET_MASS = 9.8e13f;
	constexpr float MOON_MASS = PLANET_MASS / 10.0f;


	// Distances 
	constexpr float PLANET_RADIUS = 64.0;
	constexpr float MOON_RADIUS = 24.0f;
	constexpr float SATELLITE_RADIUS = 8.0f;
	constexpr float PLAYER_ORBIT_RADIUS = 50.0f;
	constexpr float MOON_ORBIT_RADIUS = 75.0f;

	// Populations 
	constexpr Krawler::int32 PLANETS_COUNT = 5;
	constexpr Krawler::int32 NETWORKED_PLANETS_COUNT = 3;
	constexpr Krawler::int32 AI_PLANETS_COUNT = 3;
	constexpr Krawler::int32 SATELLITES_COUNT = 50;
	constexpr Krawler::int32 MOON_COUNT = 2;


	constexpr Krawler::int32 MIN_AI = 1;
	constexpr Krawler::int32 MAX_AI = 3;

	static const Krawler::Vec2f PLAYER_SATELLITE_DIMENSION { 24, 24 };
	static const Krawler::Vec2f AI_SATELLITE_DIMENSION { 24, 24 };

	// Time Values
	constexpr float SATELLITE_ALIVE_TIME = 8.0f; // 5 seconds of alive time

	constexpr float PLAYER_ENTITY_ROTATION_SPEED = 50.0f;

}