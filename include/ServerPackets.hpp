#pragma once

#include <string>
#include <SFML/Network.hpp>

#include <Krawler.h>

enum class MessageType : Krawler::int32
{
	None = -1,
	KeepAlive,
	Establish,
	Disconnect,
	Move = 20
};

#pragma region Server Client Base Struct
struct ServerClientMessage
{
	MessageType type = MessageType::None;
	long long timeStamp = 0;
};

#pragma endregion 

//--------------------------------------------------------------------------------
#pragma region Keep Alive Struct
struct KeepAlive : public ServerClientMessage
{
	KeepAlive() { type = MessageType::KeepAlive; }
	std::string message;
};

static sf::Packet& operator <<(sf::Packet& p, const KeepAlive& keepAlive)
{
	return p << static_cast<Krawler::int32>(keepAlive.type) << keepAlive.timeStamp << keepAlive.message;
}


static sf::Packet& operator >>(sf::Packet& p, KeepAlive& keepAlive)
{
	int a;
	p >> a;
	keepAlive.type = static_cast<MessageType>(a);
	return p >> keepAlive.timeStamp >> keepAlive.message;
}
#pragma endregion

//--------------------------------------------------------------------------------

#pragma region Establish Connection Struct
struct EstablishConnection : public ServerClientMessage
{
	EstablishConnection() { type = MessageType::Establish; }
	std::string clientVersion;
};

static sf::Packet& operator <<(sf::Packet& p, const EstablishConnection& establishStruct)
{
	return p << static_cast<Krawler::int32>(establishStruct.type) << establishStruct.clientVersion;
}


static sf::Packet& operator >>(sf::Packet& p, EstablishConnection& establishStruct)
{
	int a;
	p >> a;
	establishStruct.type = static_cast<MessageType>(a);
	return p >> establishStruct.clientVersion;
}
#pragma endregion

//--------------------------------------------------------------------------------
#pragma region Disconnect Connection Struct
struct DisconnectConnection : public ServerClientMessage
{
	DisconnectConnection() { type = MessageType::Disconnect; }
};


static sf::Packet& operator <<(sf::Packet& p, const DisconnectConnection& dc)
{
	return p << static_cast<Krawler::int32>(dc.type);
}

static sf::Packet& operator >>(sf::Packet& p, DisconnectConnection& dc)
{
	int a;
	p >> a;
	dc.type = static_cast<MessageType>(a);
	return p;
}

#pragma endregion

//--------------------------------------------------------------------------------

#pragma region MoveInWorld
struct MoveInWorld : public ServerClientMessage
{
	MoveInWorld() { type = MessageType::Move; }
	Krawler::Vec2f playerPosition;
	std::string playerName;
};

static sf::Packet& operator <<(sf::Packet& p, const MoveInWorld& moveInWorld)
{
	return p << static_cast<Krawler::int32>(moveInWorld.type) <<
		moveInWorld.playerPosition.x << moveInWorld.playerPosition.y <<
		moveInWorld.playerName << moveInWorld.timeStamp;
}

static sf::Packet& operator >>(sf::Packet& p, MoveInWorld& moveInWorld)
{
	int a;
	p >> a;
	moveInWorld.type = static_cast<MessageType>(a);
	p >> moveInWorld.playerPosition.x >> moveInWorld.playerPosition.y >> moveInWorld.playerName >> moveInWorld.timeStamp;
	return p;
}
#pragma endregion

//--------------------------------------------------------------------------------
