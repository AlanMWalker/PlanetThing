#pragma once

#include "BlockedMap.h"

class GameSetup
{
public: 
	
	GameSetup();

private: 

	void createGod();
	void createMap();
	void createBlockedMap();
	void createPlayer();
	void createAi();

	BlockedMap m_blockedMap;
	
};