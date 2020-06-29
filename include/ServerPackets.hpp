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
};

#pragma region Server Client Base Struct
struct ServerClientMessage
{
	MessageType type = MessageType::None;
	long long timeStamp = 0;
};

#pragma endregion 

static void write_base_out(sf::Packet& p, ServerClientMessage* scm)
{
	p << static_cast<Krawler::int32>(scm->type) << scm->timeStamp;
}

static void read_base_in(sf::Packet& p, ServerClientMessage* scm)
{
	int a;
	p >> a;
	scm->type = (MessageType)(a);
	p >> scm->timeStamp;
}


//--------------------------------------------------------------------------------
#pragma region Keep Alive Struct
struct KeepAlive : public ServerClientMessage
{
	KeepAlive() { type = MessageType::KeepAlive; }
	std::string message;
};

static sf::Packet& operator <<(sf::Packet& p, const KeepAlive& keepAlive)
{
	write_base_out(p, (ServerClientMessage*)&keepAlive);
	return p << keepAlive.message;
}


static sf::Packet& operator >>(sf::Packet& p, KeepAlive& keepAlive)
{
	read_base_in(p, (ServerClientMessage*)&keepAlive);
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
	write_base_out(p, (ServerClientMessage*)&establishStruct);

	return p << establishStruct.clientVersion;
}


static sf::Packet& operator >>(sf::Packet& p, EstablishConnection& establishStruct)
{
	read_base_in(p, (ServerClientMessage*)&establishStruct);
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
	write_base_out(p, (ServerClientMessage*)&dc);
	return p;
}

static sf::Packet& operator >>(sf::Packet& p, DisconnectConnection& dc)
{
	read_base_in(p, (ServerClientMessage*)&dc);
	return p;
}

#pragma endregion
