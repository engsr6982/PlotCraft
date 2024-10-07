#include "TemplateGenerator.h"
#include "TemplateManager.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/VanillaBiomeNames.h"
#include "mc/world/level/biome/registry/BiomeRegistry.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include <memory>


namespace plot::core {


TemplateGenerator::TemplateGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains);
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);

    bool ok = TemplateManager::generatorBlockVolume(mPrototype);
    if (!ok) {
        throw std::runtime_error("Failed to generatorBlockVolume");
    }
}


void TemplateGenerator::loadChunk(LevelChunk& levelchunk, bool) {
    auto& chunkPos = levelchunk.getPosition();

    levelchunk.setBlockVolume(TemplateManager::mBlockVolume[TemplateManager::calculateChunkID(chunkPos)], 0);


    levelchunk.recomputeHeightMap(0);
    mBiomeSource->fillBiomes(levelchunk, ChunkLocalNoiseCache{});
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}


} // namespace plot::core
