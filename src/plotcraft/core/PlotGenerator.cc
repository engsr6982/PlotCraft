#include "PlotGenerator.h"

#include "mc/deps/core/data/DividedPos2d.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/VanillaBiomeNames.h"
#include "mc/world/level/biome/registry/BiomeRegistry.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/PostprocessingManager.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"


namespace plotcraft::core {

PlotGenerator::PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    // 值得注意的是，我们是继承的FlatWorldGenerator，后续也会使用其内部成员，所以我们需要调用FlatWorldGenerator的构造
    random.mRandom.mObject._setSeed(seed);

    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains);
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);
}

bool PlotGenerator::postProcess(ChunkViewSource& neighborhood) {
    ChunkPos chunkPos;
    chunkPos.x      = neighborhood.getArea().mBounds.min.x;
    chunkPos.z      = neighborhood.getArea().mBounds.min.z;
    auto levelChunk = neighborhood.getExistingChunk(chunkPos);
    auto seed       = getLevel().getSeed();

    // 必须，需要给区块上锁
    auto lockChunk =
        levelChunk->getDimension().mPostProcessingManager->tryLock(levelChunk->getPosition(), neighborhood);

    if (!lockChunk) {
        return false;
    }
    BlockSource blockSource(getLevel(), neighborhood.getDimension(), neighborhood, false, true, true);
    auto        chunkPosL = levelChunk->getPosition();
    random.mRandom.mObject._setSeed(seed);
    auto one = 2 * (random.nextInt() / 2) + 1;
    auto two = 2 * (random.nextInt() / 2) + 1;
    random.mRandom.mObject._setSeed(seed ^ (chunkPosL.x * one + chunkPosL.z * two));

    // 放置结构体，如果包含有某个结构的区块，就会放置loadChunk准备的结构
    WorldGenerator::postProcessStructureFeatures(blockSource, random, chunkPosL.x, chunkPosL.z);
    // 处理其它单体结构，比如沉船，这里不是必须
    WorldGenerator::postProcessStructures(blockSource, random, chunkPosL.x, chunkPosL.z);
    return true;
}

void PlotGenerator::loadChunk(LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad) {
    auto chunkPos = levelchunk.getPosition();

    auto            blockPos = BlockPos(chunkPos, 0);
    DividedPos2d<4> dividedPos2D;
    dividedPos2D.x = (blockPos.x >> 31) - ((blockPos.x >> 31) - blockPos.x) / 4;
    dividedPos2D.z = (blockPos.z >> 31) - ((blockPos.z >> 31) - blockPos.z) / 4;

    // 处理其它单体结构，比如沉船，这里不是必须
    WorldGenerator::preProcessStructures(getDimension(), chunkPos, getBiomeSource());
    // 准备要放置的结构，如果是某个结构的区块，就会准备结构
    WorldGenerator::prepareStructureFeatureBlueprints(getDimension(), chunkPos, getBiomeSource(), *this);

    // 这里并没有放置结构，只有单纯基本地形
    levelchunk.setBlockVolume(mPrototype, 0);

    levelchunk.recomputeHeightMap(0);
    ChunkLocalNoiseCache chunkLocalNoiseCache(dividedPos2D, 8);
    mBiomeSource->fillBiomes(levelchunk, chunkLocalNoiseCache);
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}

std::optional<short> PlotGenerator::getPreliminarySurfaceLevel(DividedPos2d<4> worldPos) const {
    // 超平坦的高度都是一样的，直接返回固定值即可
    return -61;
}

void PlotGenerator::prepareAndComputeHeights(
    BlockVolume&        box,
    ChunkPos const&     chunkPos,
    std::vector<short>& ZXheights,
    bool                factorInBeardsAndShavers,
    int                 skipTopN
) {
    auto heightMap = mPrototype.computeHeightMap();
    ZXheights.assign(heightMap->begin(), heightMap->end());
}

void PlotGenerator::prepareHeights(BlockVolume& box, ChunkPos const& chunkPos, bool factorInBeardsAndShavers) {
    // 在其它类型世界里，这里是需要对box进行处理，生成地形，超平坦没有这个需要，所以直接赋值即可
    box = mPrototype;
};

StructureFeatureType PlotGenerator::findStructureFeatureTypeAt(BlockPos const& blockPos) {
    return WorldGenerator::findStructureFeatureTypeAt(blockPos);
};

bool PlotGenerator::isStructureFeatureTypeAt(const BlockPos& blockPos, ::StructureFeatureType type) const {
    return WorldGenerator::isStructureFeatureTypeAt(blockPos, type);
}

bool PlotGenerator::findNearestStructureFeature(
    ::StructureFeatureType      type,
    BlockPos const&             blockPos,
    BlockPos&                   blockPos1,
    bool                        mustBeInNewChunks,
    std::optional<HashedString> hash
) {
    return WorldGenerator::findNearestStructureFeature(type, blockPos, blockPos1, mustBeInNewChunks, hash);
};

void PlotGenerator::garbageCollectBlueprints(buffer_span<ChunkPos> activeChunks) {
    return WorldGenerator::garbageCollectBlueprints(activeChunks);
};

} // namespace plotcraft::core
