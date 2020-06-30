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
	GeneratedLevel
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
};

static sf::Packet& operator <<(sf::Packet& p, const KeepAlive& keepAlive)
{
	write_base_out(p, (ServerClientMessage*)&keepAlive);
	return p;
}


static sf::Packet& operator >>(sf::Packet& p, KeepAlive& keepAlive)
{
	read_base_in(p, (ServerClientMessage*)&keepAlive);
	return p;
}
#pragma endregion

//--------------------------------------------------------------------------------

#pragma region Establish Connection Struct
struct EstablishConnection : public ServerClientMessage
{
	EstablishConnection() { type = MessageType::Establish; }
	std::string clientVersion;
	std::string displayName;
};

static sf::Packet& operator <<(sf::Packet& p, const EstablishConnection& establishStruct)
{
	write_base_out(p, (ServerClientMessage*)&establishStruct);
	return p << establishStruct.clientVersion << establishStruct.displayName;
}


static sf::Packet& operator >>(sf::Packet& p, EstablishConnection& establishStruct)
{
	read_base_in(p, (ServerClientMessage*)&establishStruct);
	return p >> establishStruct.clientVersion >> establishStruct.displayName;
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

//--------------------------------------------------------------------------------
#pragma region Lobby Name List Struct
struct LobbyNameList : public ServerClientMessage
{
	LobbyNameList() { type = MessageType::LobbyNameList; }
	std::string nameList; //csv list
};


static sf::Packet& operator <<(sf::Packet& p, const LobbyNameList& lnl)
{
	write_base_out(p, (ServerClientMessage*)&lnl);
	return p << lnl.nameList;
}

static sf::Packet& operator >>(sf::Packet& p, LobbyNameList& lnl)
{
	read_base_in(p, (ServerClientMessage*)&lnl);
	return p >> lnl.nameList;
}

#pragma endregion

//--------------------------------------------------------------------------------
#pragma region Level Gen Struct
struct GeneratedLevel : public ServerClientMessage
{
	// Planet Positions & Masses
	// Planet & Player Pairings
	GeneratedLevel() { type = MessageType::GeneratedLevel; }

	Krawler::uint64 numOfPlanets = 0;

	std::vector<Krawler::Vec2f> positions;
	std::vector<float> masses;
	std::vector<std::string> names;
};

// Write out in this order
// Num of planets
// Planet positions 
// Planet masses
// Display Name Owners
static sf::Packet& operator <<(sf::Packet& p, const GeneratedLevel& genLevel)
{
	write_base_out(p, (ServerClientMessage*)&genLevel);
	p << genLevel.numOfPlanets;

	const Krawler::uint64 PosBufferSize = sizeof(Krawler::Vec2f) * genLevel.numOfPlanets;
	const Krawler::uint64 MassBufferSize = sizeof(float) * genLevel.numOfPlanets;

	char* positionBuffer = new char[PosBufferSize];
	KCHECK(positionBuffer);

	char* massBuffer = new char[MassBufferSize];
	KCHECK(massBuffer);

	memcpy_s((void*)positionBuffer, PosBufferSize, (void*)&genLevel.positions[0], PosBufferSize);
	memcpy_s((void*)massBuffer, MassBufferSize, (void*)&genLevel.masses[0], MassBufferSize);

	p.append(positionBuffer, PosBufferSize);
	p.append(massBuffer, MassBufferSize);

	for (auto& name : genLevel.names)
	{
		p.append(name.c_str(), name.length());
	}

	delete[] positionBuffer;
	delete[] massBuffer;
	return p;
}

static sf::Packet& operator >>(sf::Packet& p, GeneratedLevel& genLevel)
{
	read_base_in(p, (ServerClientMessage*)&genLevel);
	// Addon the offset for the number of planets var into the struct
	
	char* pData = (char*)p.getData() + sizeof(MessageType) + sizeof(long long);

	memcpy_s(&genLevel.numOfPlanets, sizeof(Krawler::uint64), pData, sizeof(Krawler::uint64));
	genLevel.numOfPlanets = _byteswap_uint64(genLevel.numOfPlanets);

	const Krawler::uint64 PosBufferSize = sizeof(Krawler::Vec2f) * genLevel.numOfPlanets;
	const Krawler::uint64 MassBufferSize = sizeof(float) * genLevel.numOfPlanets;

	genLevel.positions.resize(genLevel.numOfPlanets);
	genLevel.masses.resize(genLevel.numOfPlanets);
	genLevel.names.resize(genLevel.numOfPlanets);

	const Krawler::uint64 PositionOffset = sizeof(Krawler::uint64);
	const Krawler::uint64 MassOffset = sizeof(Krawler::uint64) + PosBufferSize;
	const Krawler::uint64 NamesOffset = sizeof(Krawler::uint64) + PosBufferSize + MassBufferSize;

	memcpy_s((void*)&genLevel.positions[0], PosBufferSize, (void*)(pData + PositionOffset), PosBufferSize);
	memcpy_s((void*)&genLevel.masses[0], MassBufferSize, (void*)(pData + MassOffset), MassBufferSize);

	std::string tempName;

	for (Krawler::uint64 i = 0; i < genLevel.numOfPlanets; ++i)
	{
		tempName = (pData + NamesOffset);
		genLevel.names[i] = tempName;
	}
	return p;
}

#pragma endregion