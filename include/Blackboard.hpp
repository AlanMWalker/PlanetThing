#pragma once
#include "Krawler.h"

namespace Blackboard
{
	 const float G = 6.67e-11;
	 
	 // Masses
	 constexpr float OBJECT_MASS = 200.0f;
	 constexpr float PLANET_MASS = 9.8e13f;
	 
	 // Distances 
	 constexpr float PLANET_RADIUS = 64.0f;
	 constexpr float OBJECT_RADIUS = 8.0f;
	 constexpr float PLAYER_ORBIT_RADIUS = 50.0f;

	 // Populations 
	 constexpr Krawler::int32 PLANETS_COUNT = 2;
	 constexpr Krawler::int32 OBJECTS_COUNT = 20;
	 
}