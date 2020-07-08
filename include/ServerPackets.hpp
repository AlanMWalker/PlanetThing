#pragma once

#include <string>
#include <SFML/Network.hpp>
#include <stdint.h>

#include <Krawler.h>
#include <vector>
#include <memory.h>

enum class MessageType : Krawler::int32
{
	None = -1,
	KeepAlive,
	Establish,
	Disconnect,
	LobbyNameList,
	GeneratedLevel,
	NextPlayerTurn,
	MoveSatellite, // Sent by client wanting to move
	SatellitePositionUpdate, // Sent by host once it moves a client
	FireRequest, // sent by client wanting to fire
	FireActivated // sent by server accepting fire
};

struct ServerClientMessage
{
	MessageType type = MessageType::None;
	long long timeStamp = 0;
};

//--------------------------------------------------------------------------------
struct KeepAlive : public ServerClientMessage
{
	KeepAlive() { type = MessageType::KeepAlive; }
};

sf::Packet& operator <<(sf::Packet& p, const KeepAlive& keepAlive);
sf::Packet& operator >>(sf::Packet& p, KeepAlive& keepAlive);

//--------------------------------------------------------------------------------
struct EstablishConnection : public ServerClientMessage
{
	EstablishConnection() { type = MessageType::Establish; }
	std::string clientVersion;
	std::string displayName;
	std::string uuid; // filled out on reply
};

sf::Packet& operator <<(sf::Packet& p, const EstablishConnection& establishStruct);
sf::Packet& operator >>(sf::Packet& p, EstablishConnection& establishStruct);

//--------------------------------------------------------------------------------
struct DisconnectConnection : public ServerClientMessage
{
	DisconnectConnection() { type = MessageType::Disconnect; }
};

sf::Packet& operator <<(sf::Packet& p, const DisconnectConnection& dc);
sf::Packet& operator >>(sf::Packet& p, DisconnectConnection& dc);

//--------------------------------------------------------------------------------
struct LobbyNameList : public ServerClientMessage
{
	LobbyNameList() { type = MessageType::LobbyNameList; }
	std::string nameList; //csv list
};

sf::Packet& operator <<(sf::Packet& p, const LobbyNameList& lnl);
sf::Packet& operator >>(sf::Packet& p, LobbyNameList& lnl);

//--------------------------------------------------------------------------------
struct GeneratedLevel : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	GeneratedLevel() { type = MessageType::GeneratedLevel; }

	Krawler::uint64 numOfPlanets = 0;

	std::vector<Krawler::Vec2f> positions;
	std::vector<float> masses;
	std::vector<std::string> names;
	std::vector<std::string> uuids;
};


sf::Packet& operator <<(sf::Packet& p, const GeneratedLevel& genLevel);
sf::Packet& operator >>(sf::Packet& p, GeneratedLevel& genLevel);

//--------------------------------------------------------------------------------
struct NextPlayerTurn : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	NextPlayerTurn() { type = MessageType::NextPlayerTurn; }
};

sf::Packet& operator <<(sf::Packet& p, const NextPlayerTurn& tt);
sf::Packet& operator >>(sf::Packet& p, NextPlayerTurn& tt);

//--------------------------------------------------------------------------------
struct MoveSatellite : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	MoveSatellite() { type = MessageType::MoveSatellite; }
	
	// -1 = Anticlockwise
	// +1 = Clockwise
	Krawler::int32 direction = 0;
	std::string uuid;
};

sf::Packet& operator <<(sf::Packet& p, const MoveSatellite& ms);
sf::Packet& operator >>(sf::Packet& p, MoveSatellite& ms);

//--------------------------------------------------------------------------------
struct SatellitePositionUpdate : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	SatellitePositionUpdate() { type = MessageType::SatellitePositionUpdate; }

	float theta;
	std::string uuid;
};

sf::Packet& operator <<(sf::Packet& p, const SatellitePositionUpdate& spu);
sf::Packet& operator >>(sf::Packet& p, SatellitePositionUpdate& spu);

//--------------------------------------------------------------------------------
struct FireRequest : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	FireRequest() { type = MessageType::FireRequest; }
	std::string uuid;
	float strength;
};

sf::Packet& operator <<(sf::Packet& p, const FireRequest& fr);
sf::Packet& operator >>(sf::Packet& p, FireRequest& fr);


//--------------------------------------------------------------------------------
struct FireActivated : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	FireActivated() { type = MessageType::FireActivated; }
	std::string uuid;
	float strength;
};

sf::Packet& operator <<(sf::Packet& p, const FireActivated& fa);
sf::Packet& operator >>(sf::Packet& p, FireActivated& fa);