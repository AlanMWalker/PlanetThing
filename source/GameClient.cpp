#include "GameClient.hpp"

GameClient::GameClient(const sf::IpAddress& ip, unsigned short portNum)
	: ip(ip), port(portNum), bStillConnected(false), bConnectionEstablished(false)
{
}

bool GameClient::operator==(const GameClient& rhs) const
{
	return (ip == rhs.ip) && (rhs.port == port) &&
		(bStillConnected == rhs.bStillConnected) &&
		(bConnectionEstablished == rhs.bConnectionEstablished);
}
