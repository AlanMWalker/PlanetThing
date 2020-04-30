#pragma once
#pragma once
#include <string>
#include <vector>

#include <Krawler.h>

class BlockedMap
{
public:

	BlockedMap() = default;
	~BlockedMap() = default;

	void setup(const std::wstring& level);
	void destroy();

private:
	
	struct TilesetBlockedRecord
	{
		std::vector<Krawler::int32> blockedTileIds;
		std::wstring tilesetName;
		int firstGID;
	};

	enum TileWalkState
	{
		Walkable = 0, 
		Blocked = 1
	};

	std::vector<TilesetBlockedRecord> m_blockedRecords;

	std::vector<TileWalkState> m_blockedMap;
	std::wstring m_levelName;
	
	
	void setupBlockedRecords();
	void setupBlockedMap();
};
