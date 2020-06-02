#pragma once
#pragma once
#include <string>
#include <vector>

#include <Krawler.h>
#include <KEntity.h>
#include <Tiled\KTiledImport.h>

#include "PathFinder/PathFinder.h"
#include "PathFinder/AStar.h"


class BlockedMap
{
public:
	static BlockedMap& getInstance()
	{
		static BlockedMap* pBlockedMap = new BlockedMap();

		return *pBlockedMap;
	}
	~BlockedMap() = default;

	void setup(const std::wstring& level, Krawler::KEntity* pMapEntity);
	void destroy();

	Krawler::Vec2f getTileSize() const { return m_tileSize; }

	std::vector<Krawler::Vec2f> getWalkablePath(const Krawler::Vec2f& start, const Krawler::Vec2f& end);
	std::vector<Krawler::Vec2f> getTileChildren(const Krawler::Vec2f worldPosition);


private:
	BlockedMap() = default;

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


	class TerrainNode : public AStarNode
	{
	public:
		// Used exclusively when appending child 
		// nodes to a node. These are all 8 directional nodes
		// surrounding a node, unless it is on the edge or a corner
		// node, then it will have < 8 children.
		float localDistanceTo(AStarNode* pNode)
		{
			// if it's a diagnonal child, it's 1.5 x cost
			if (pNode->getX() != getX() && pNode->getY() != getY())
			{
				return 1.41421356237f;
			}

			// otherwise we'll return 1 since the node is adjacent 
			// either vertically or horizontally
			return 1.0f;
		}

		virtual float distanceTo(AStarNode* pNode) const override
		{
			const Krawler::Vec2f myPos(getX(), getY());
			const Krawler::Vec2f theirPos(pNode->getX(), pNode->getY());
			const Krawler::Vec2f dx = theirPos - myPos;
			return GetLength(dx);
		}
	};

	void setupBlockedRecords();
	void setupBlockedMap();
	void setupTerrainNodes();

	std::vector<TilesetBlockedRecord> m_blockedRecords;
	std::vector<TileWalkState> m_blockedMap;
	std::vector<TerrainNode> m_terrainNodes;
	std::vector<TerrainNode*> m_foundPath;
	std::vector<Krawler::Vec2f> m_transformedPath;

	std::wstring m_levelName;
	Krawler::Vec2f m_tileSize;
	PathFinder<TerrainNode> m_pathFinder; // Used to find a path in a MyNode graph

	Krawler::KEntity* m_pMapEntity = nullptr;
	Krawler::TiledImport::KTIMap* m_pLevelImportData = nullptr;
};
