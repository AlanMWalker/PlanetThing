#include "BlockedMap.h"

#include <AssetLoader\KAssetLoader.h>	
#include <Tiled\KTiledImport.h>

using namespace Krawler;
using namespace Krawler::TiledImport;

void BlockedMap::setup(const std::wstring& level)
{
	auto levelImportData = ASSET().getLevelMap(level);
	auto tileset = levelImportData->tilesetVector.back();

	
	// Uncomment when switching to multiple tileset support
	/*for (auto tileset : levelImportData->tilesetVector)
	{

	}*/

}
