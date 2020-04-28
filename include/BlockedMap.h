#pragma once
#include <string>
#include <vector>

class BlockedMap
{
public: 

	BlockedMap() = default;
	~BlockedMap() = default;

	void setup(const std::wstring& level);

private:
	
	std::vector<int> m_blockedTileIds;

};
