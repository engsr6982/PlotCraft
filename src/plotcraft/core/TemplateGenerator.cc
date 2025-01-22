#include "TemplateGenerator.h"
#include "TemplateManager.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/registry/BiomeRegistry.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include <mc\world\level\biome\registry\BiomeRegistry.h>
#include <mc\world\level\biome\registry\VanillaBiomeNames.h>
#include <mc\world\level\biome\source\FixedBiomeSource.h>
#include <mc\world\level\block\VanillaBlockTypeIds.h>
#include <mc\world\level\dimension\Dimension.h>
#include <memory>


namespace plot::core {


TemplateGenerator::TemplateGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains());
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);

    bool ok = TemplateManager::generatorBlockVolume(mPrototype);
    if (!ok) {
        throw std::runtime_error("Failed to generatorBlockVolume");
    }
}


void TemplateGenerator::loadChunk(LevelChunk& levelchunk, bool) {
    auto& chunkPos = levelchunk.getPosition();

    auto iter = TemplateManager::mBlockVolume.find(TemplateManager::calculateChunkID(chunkPos));
    if (iter == TemplateManager::mBlockVolume.end()) {
        throw std::runtime_error("Failed to find chunk in TemplateManager");
    }
    levelchunk.setBlockVolume(iter->second, 0);


    levelchunk.recomputeHeightMap(0);
    mBiomeSource->fillBiomes(levelchunk, ChunkLocalNoiseCache{});
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}


} // namespace plot::core
