#include "BlockedMap.h"

#include <AssetLoader\KAssetLoader.h>	
#include <KScene.h>
#include <KApplication.h>
#include <Components\KCBoxCollider.h>



using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::TiledImport;

// Public

void BlockedMap::setup(const std::wstring& level, Krawler::KEntity* pMapEntity)
{
	m_levelName = level;
	m_pMapEntity = pMapEntity;
	m_pLevelImportData = ASSET().getLevelMap(m_levelName);
	setupBlockedRecords();
	setupBlockedMap();
	setupTerrainNodes();
}

void BlockedMap::destroy()
{
	m_blockedMap.clear();
	m_blockedRecords.clear();
	m_levelName = L"";
}

std::vector<Vec2f> BlockedMap::getWalkablePath(const Krawler::Vec2f& start, const Krawler::Vec2f& end)
{
	m_foundPath.clear();
	m_transformedPath.clear();

	Vec2i startTile((int32)(start.x / m_tileSize.x), (int32)(start.y / m_tileSize.y));
	Vec2i endTile((int32)(end.x / m_tileSize.x), (int32)(end.y / m_tileSize.y));

	if (startTile.x < 0 || startTile.x > m_pLevelImportData->width || startTile.y < 0 || startTile.y > m_pLevelImportData->height)
	{
		return m_transformedPath;
	}

	if (endTile.x < 0 || endTile.x > m_pLevelImportData->width || endTile.y < 0 || endTile.y > m_pLevelImportData->height)
	{
		return m_transformedPath;
	}

	m_pathFinder.setStart(m_terrainNodes[startTile.x + startTile.y * m_pLevelImportData->width]);
	m_pathFinder.setGoal(m_terrainNodes[endTile.x + endTile.y * m_pLevelImportData->width]);

	bool bCanFindPath = m_pathFinder.findPath<AStar>(m_foundPath);
	AStar::getInstance().clear();

	if (bCanFindPath)
	{
		KPRINTF("Can find path\n");
	}
	else
	{
		KPRINTF("Can't find path\n");
	}

	for (auto& p : m_foundPath)
	{
		const int32 x = p->getX();
		const int32 y = p->getY();
		const Vec2f centre((x * m_tileSize.x) + (m_tileSize.x * 0.5f), (y * m_tileSize.y) + (m_tileSize.y * 0.5f));

		m_transformedPath.push_back(centre);
	}

	return m_transformedPath;
}

std::vector<Vec2f> BlockedMap::getTileChildren(const Vec2f worldPosition)
{
	std::vector<Vec2f> childPositions;

	const Vec2i TilePosition((int32)(worldPosition.x / m_tileSize.x), (int32)(worldPosition.y / m_tileSize.y));

	if (TilePosition.x < 0 || TilePosition.x > m_pLevelImportData->width ||
		TilePosition.y < 0 || TilePosition.y > m_pLevelImportData->height)
	{
		return childPositions;
	}

	childPositions.push_back(Vec2f(TilePosition.x * m_tileSize.x, TilePosition.y * m_tileSize.y));

	auto tile = m_terrainNodes[TilePosition.x + TilePosition.y * m_pLevelImportData->width];
	const auto& children = tile.getChildren();

	for (auto c : children)
	{
		auto child = (TerrainNode*)c.first;
		const Vec2f pos(child->getX() * m_tileSize.x, child->getY() * m_tileSize.y);
		childPositions.push_back(pos);
	}

	return childPositions;
}

// Private 

void BlockedMap::setupBlockedRecords()
{
	auto tileset = m_pLevelImportData->tilesetVector.back();
	TilesetBlockedRecord record;

	for (auto tileset : m_pLevelImportData->tilesetVector)
	{
		record.firstGID = tileset.firstGID;
		record.tilesetName = tileset.name;
		for (auto& tilePropList : tileset.tilePropertiesMap)
		{
			for (auto& prop : tilePropList.second)
			{
				// if we've found that this tile has a blocked property
				if (prop.first == KTEXT("blocked"))
				{
					if (prop.second.type_bool)
					{
						const int32 tileID = (_wtoi(tilePropList.first.c_str()));
						record.blockedTileIds.push_back(tileID);

					}
				}
			}
		}
		m_blockedRecords.push_back(record);
	}
}

void BlockedMap::setupBlockedMap()
{
	// May be more optimal to use a set internally 
	// within the TilesetBlockedRecord struct
	m_tileSize = Vec2f(m_pLevelImportData->tileWidth, m_pLevelImportData->tileHeight);
	m_tileSize.x *= m_pMapEntity->getTransform()->getScale().x;
	m_tileSize.y *= m_pMapEntity->getTransform()->getScale().y;

	// Scale tilesize by tilemap 

	std::wstring mapPrint;
	bool bIsFirstTileLayer = true;
	for (auto& layer : m_pLevelImportData->layersVector)
	{
		if (layer.layerType != KTILayerTypes::TileLayer)
		{
			continue;
		}

		for (int32 j = 0; j < m_pLevelImportData->height; ++j)
		{
			for (int32 i = 0; i < m_pLevelImportData->width; ++i)
			{

				const int32 tileIdx = (i + j * layer.width);
				// Below only functions for 1 tileset, does not account for multiple
				const int32 localID = layer.tileData[tileIdx] - m_blockedRecords.back().firstGID;
				auto result = std::find(m_blockedRecords.back().blockedTileIds.begin(), m_blockedRecords.back().blockedTileIds.end(), localID);
				if (result == m_blockedRecords.back().blockedTileIds.end())
				{
					if (bIsFirstTileLayer)
					{
						m_blockedMap.push_back(TileWalkState::Walkable);
						mapPrint.push_back('_');
					}
					else
					{
						if (m_blockedMap[tileIdx] != TileWalkState::Walkable)
						{
							m_blockedMap[tileIdx] = TileWalkState::Walkable;
							mapPrint[tileIdx] = '_';
						}

					}
				}
				else
				{
					if (bIsFirstTileLayer)
					{
						m_blockedMap.push_back(TileWalkState::Blocked);
						addBoxCollider(i, j);
						mapPrint.push_back('#');
					}
					else
					{
						if (m_blockedMap[tileIdx] != TileWalkState::Blocked)
						{
							m_blockedMap[tileIdx] = TileWalkState::Blocked;
							addBoxCollider(i, j);
							mapPrint[tileIdx] = '#';
						}
					}
				}
			}
		}
		bIsFirstTileLayer = false;

	}

	for (int32 j = 0; j < m_pLevelImportData->height; ++j)
	{
		for (int32 i = 0; i < m_pLevelImportData->width; ++i)
		{
			wprintf_s(L"%c", mapPrint[i + j * m_pLevelImportData->width]);
		}
		wprintf_s(L"\n");

	}
}

void BlockedMap::setupTerrainNodes()
{
	int32 tempX = 0, tempY = 0;

	for (int32 j = 0; j < m_pLevelImportData->height; ++j)
	{
		for (int32 i = 0; i < m_pLevelImportData->width; ++i)
		{
			TerrainNode node;
			node.setPosition(i, j);
			m_terrainNodes.push_back(node);
		}
	}

	for (int32 j = 0; j < m_pLevelImportData->height; ++j)
	{
		for (int32 i = 0; i < m_pLevelImportData->width; ++i)
		{
			auto& terrainNode = m_terrainNodes[i + j * m_pLevelImportData->width];
			// Delta j
			for (int32 dj = -1; dj < 2; ++dj)
			{
				tempY = terrainNode.getY() + dj;

				// Delta i
				for (int32 di = -1; di < 2; ++di)
				{
					tempX = terrainNode.getX() + di;

					if (tempX > -1 && tempX < m_pLevelImportData->width && tempY > -1 && tempY < m_pLevelImportData->height) // be sure not to go outside the limits
					{
						auto& aChild = (m_terrainNodes[tempX + tempY * m_pLevelImportData->width]);

						if (m_blockedMap[tempX + tempY * m_pLevelImportData->width] == TileWalkState::Walkable && (tempX != terrainNode.getX() || tempY != terrainNode.getY()))
						{
							terrainNode.addChild(&aChild, terrainNode.localDistanceTo(&aChild));
						}
					}
				}
			}
		}
	}
}

void BlockedMap::addBoxCollider(Krawler::int32 i, Krawler::int32 j)
{
	auto pEntity = GET_SCENE()->addEntityToScene();
	KCBoxCollider & collider = *new KCBoxCollider(pEntity, m_tileSize);
	pEntity->setTag(L"Terrain");
	pEntity->addComponent(&collider);
	pEntity->getTransform()->setPosition(i * m_tileSize.x, j * m_tileSize.y);
	pEntity->getTransform()->setOrigin(m_tileSize * 0.5f);
	KCColliderFilteringData data;
	data.collisionFilter = 0x0000;
	data.collisionMask = 0x0001;

	collider.setCollisionFilteringData(data);
}
