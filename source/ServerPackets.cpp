#include "ServerPackets.hpp"

void write_base_out(sf::Packet& p, ServerClientMessage* scm)
{
	p << static_cast<Krawler::int32>(scm->type) << scm->timeStamp;
}

void read_base_in(sf::Packet& p, ServerClientMessage* scm)
{
	int a;
	p >> a;
	scm->type = (MessageType)(a);
	p >> scm->timeStamp;
}

sf::Packet& operator<<(sf::Packet& p, const KeepAlive& keepAlive)
{
	write_base_out(p, (ServerClientMessage*)&keepAlive);
	return p;
}

sf::Packet& operator>>(sf::Packet& p, KeepAlive& keepAlive)
{
	read_base_in(p, (ServerClientMessage*)&keepAlive);
	return p;
}
sf::Packet& operator<<(sf::Packet& p, const EstablishConnection& establishStruct)
{
	write_base_out(p, (ServerClientMessage*)&establishStruct);
	return p << establishStruct.clientVersion << establishStruct.displayName;
}

sf::Packet& operator>>(sf::Packet& p, EstablishConnection& establishStruct)
{
	read_base_in(p, (ServerClientMessage*)&establishStruct);
	return p >> establishStruct.clientVersion >> establishStruct.displayName;
}

sf::Packet& operator<<(sf::Packet& p, const DisconnectConnection& dc)
{
	write_base_out(p, (ServerClientMessage*)&dc);
	return p;
}

sf::Packet& operator>>(sf::Packet& p, DisconnectConnection& dc)
{
	read_base_in(p, (ServerClientMessage*)&dc);
	return p;
}

sf::Packet& operator<<(sf::Packet& p, const LobbyNameList& lnl)
{
	write_base_out(p, (ServerClientMessage*)&lnl);
	return p << lnl.nameList;
}

sf::Packet& operator>>(sf::Packet& p, LobbyNameList& lnl)
{
	read_base_in(p, (ServerClientMessage*)&lnl);
	return p >> lnl.nameList;
}
// Write out in this order
// Num of planets
// Planet positions 
// Planet masses
// Display Name Owners
sf::Packet& operator<<(sf::Packet& p, const GeneratedLevel& genLevel)
{
	write_base_out(p, (ServerClientMessage*)&genLevel);
	p << genLevel.numOfPlanets;

	const Krawler::uint64 PosBufferSize = sizeof(Krawler::Vec2f) * genLevel.numOfPlanets;
	const Krawler::uint64 MassBufferSize = sizeof(float) * genLevel.numOfPlanets;

	p.append((void*)&genLevel.positions[0], PosBufferSize);
	p.append((void*)&genLevel.masses[0], MassBufferSize);

	for (auto name : genLevel.names)
	{
		name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
		p.append(name.c_str(), name.length());
		p << (Krawler::uint8)('\0');
	}

	return p;
}

sf::Packet& operator>>(sf::Packet& p, GeneratedLevel& genLevel)
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
	Krawler::uint64 summedSize = 0;
	for (Krawler::uint64 i = 0; i < genLevel.numOfPlanets; ++i)
	{
		// if it's the first name it will be located at pData+NamesOffset
		// otherwise it will be located at pData + NamesOffset + SummedStringSize
		tempName = (pData + NamesOffset + summedSize);
		summedSize += tempName.size() + 1; // +1 for '\0' 
		genLevel.names[i] = tempName;
	}

	return p;
}

sf::Packet& operator<<(sf::Packet& p, const NextPlayerTurn& gm)
{
	write_base_out(p, (ServerClientMessage*)&gm);
	return p;
}

sf::Packet& operator>>(sf::Packet& p, NextPlayerTurn& gm)
{
	read_base_in(p, (ServerClientMessage*)&gm);
	return p;
}