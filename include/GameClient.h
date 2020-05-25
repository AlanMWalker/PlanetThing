#pragma once

#include <chrono>

#include <SFML\Network.hpp>
#include <SFML\System.hpp>

struct GameClient
{
	GameClient(const sf::IpAddress& ip, unsigned short portNum);
	~GameClient() = default;

	bool operator==(const GameClient& rhs)const;

	sf::IpAddress ip;
	unsigned short port;


	bool bStillConnected;
	bool bConnectionEstablished;
	std::chrono::system_clock::time_point lastPollTime;
};