#include "BlockedMap.h"

#include <AssetLoader\KAssetLoader.h>	
#include <Tiled\KTiledImport.h>
#include <KScene.h>
#include <KApplication.h>
#include <Components\KCBoxCollider.h>

using namespace Krawler;
using namespace Krawler::Components;
using namespace Krawler::TiledImport;

// Public

void BlockedMap::setup(const std::wstring& level)
{
	m_levelName = level;
	setupBlockedRecords();
	setupBlockedMap();
}

void BlockedMap::destroy()
{
	m_blockedMap.clear();
	m_blockedRecords.clear();
	m_levelName = L"";
}

// Private 

void BlockedMap::setupBlockedRecords()
{
	auto levelImportData = ASSET().getLevelMap(m_levelName);
	auto tileset = levelImportData->tilesetVector.back();
	TilesetBlockedRecord record;

	for (auto tileset : levelImportData->tilesetVector)
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
	auto levelImportData = ASSET().getLevelMap(m_levelName);
	for (int32 j = 0; j < levelImportData->height; ++j)
	{
		for (int32 i = 0; i < levelImportData->width; ++i)
		{
			for (auto& layer : levelImportData->layersVector)
			{
				if (layer.layerType != KTILayerTypes::TileLayer)
				{
					continue;
				}
				const int32 tileIdx = (i + j * layer.width);
				// Below only functions for 1 tileset, does not account for multiple
				const int32 localID = layer.tileData[tileIdx] - m_blockedRecords.back().firstGID;
				auto result = std::find(m_blockedRecords.back().blockedTileIds.begin(), m_blockedRecords.back().blockedTileIds.end(), localID);
				if (result == m_blockedRecords.back().blockedTileIds.end())
				{
					m_blockedMap.push_back(TileWalkState::Walkable);
					KPRINTF("_");
				}
				else
				{
					m_blockedMap.push_back(TileWalkState::Blocked);
					auto pEntity = GET_SCENE()->addEntityToScene();
					pEntity->setTag(L"Terrain");
					const Vec2f tileDim = Vec2f(levelImportData->tileWidth, levelImportData->tileHeight);
					pEntity->addComponent(new KCBoxCollider(pEntity, tileDim));
					pEntity->getTransform()->setPosition(i * tileDim.x, j * tileDim.y);
					pEntity->getTransform()->setOrigin(tileDim * 0.5f);
					KPRINTF("#");
				}
			}
		}
		KPRINTF("\n");
	}
}
