#include "PlotGenerator.h"
#include "plotcraft/config/Config.h"

#include "fmt/format.h"
#include "mc/deps/core/data/DividedPos2d.h"
#include "mc/network/packet/UpdateSubChunkBlocksPacket.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/VanillaBiomeNames.h"
#include "mc/world/level/biome/registry/BiomeRegistry.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/PostprocessingManager.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include <cstdio>


namespace plo::core {

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
    // auto seed       = getLevel().getSeed();

    // // 必须，需要给区块上锁
    auto lockChunk =
        levelChunk->getDimension().mPostProcessingManager->tryLock(levelChunk->getPosition(), neighborhood);

    if (!lockChunk) {
        return false;
    }
    // BlockSource blockSource(getLevel(), neighborhood.getDimension(), neighborhood, false, true, true);
    // auto        chunkPosL = levelChunk->getPosition();
    // random.mRandom.mObject._setSeed(seed);
    // auto one = 2 * (random.nextInt() / 2) + 1;
    // auto two = 2 * (random.nextInt() / 2) + 1;
    // random.mRandom.mObject._setSeed(seed ^ (chunkPosL.x * one + chunkPosL.z * two));

    // 放置结构体，如果包含有某个结构的区块，就会放置loadChunk准备的结构
    // WorldGenerator::postProcessStructureFeatures(blockSource, random, chunkPosL.x, chunkPosL.z);
    // 处理其它单体结构，比如沉船，这里不是必须
    // WorldGenerator::postProcessStructures(blockSource, random, chunkPosL.x, chunkPosL.z);
    return true;
}


// Generator Plot

int positiveMod(int value, int modulus) {
    int result = value % modulus;
    if (result < 0) {
        result += modulus;
    }
    return result;
}


void PlotGenerator::loadChunk(LevelChunk& levelchunk, bool /* forceImmediateReplacementDataLoad */) {
    auto& gen = config::cfg.generator;
    // 方块配置
    const Block& roadBlock   = *Block::tryGetFromRegistry(gen.roadBlock, 0);
    const Block& fillBlock   = *Block::tryGetFromRegistry(gen.fillBlock, 0);
    const Block& borderBlock = *Block::tryGetFromRegistry(gen.borderBlock, 0);

    // 生成/计算
    auto& chunkPos = levelchunk.getPosition();

    auto blockSource = &getDimension().getBlockSourceFromMainChunkSource();
    levelchunk.setBlockVolume(mPrototype, 0); // 设置基础地形

    // 计算当前区块的全局坐标
    int startX = chunkPos.x * 16;
    int startZ = chunkPos.z * 16;

    // 遍历区块内的每个方块位置
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            // 计算全局坐标
            int globalX = startX + x;
            int globalZ = startZ + z;

            // 计算在地盘网格中的位置
            int gridX = positiveMod(globalX, gen.plotWidth + gen.roadWidth); // 地皮 + 道路宽度
            int gridZ = positiveMod(globalZ, gen.plotWidth + gen.roadWidth);

            // 判断是否为道路或边框
            if (gridX >= gen.plotWidth || gridZ >= gen.plotWidth) {
                // 道路
                levelchunk
                    .setBlock(ChunkBlockPos{BlockPos(x, gen.generatorY, z), -64}, roadBlock, blockSource, nullptr);
            } else if (gridX == gen.roadWidth - 3 || gridZ == gen.roadWidth - 3 || gridX == gen.plotWidth - 1 || gridZ == gen.plotWidth - 1) {
                // 边框
                levelchunk.setBlock(
                    ChunkBlockPos{BlockPos(x, gen.generatorY + 1, z), -64},
                    borderBlock,
                    blockSource,
                    nullptr
                );
            } else {
                // 地皮内部
                levelchunk
                    .setBlock(ChunkBlockPos{BlockPos(x, gen.generatorY, z), -64}, fillBlock, blockSource, nullptr);
            }
        }
    }

    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}

std::optional<short> PlotGenerator::getPreliminarySurfaceLevel(DividedPos2d<4> /* worldPos */) const {
    return plo::config::cfg.generator.generatorY + 1; // 生成层 + 1
}

void PlotGenerator::prepareAndComputeHeights(
    BlockVolume& /* box */,
    ChunkPos const& /* chunkPos */,
    std::vector<short>& ZXheights,
    bool /* factorInBeardsAndShavers */,
    int /* skipTopN */
) {
    auto heightMap = mPrototype.computeHeightMap();
    ZXheights.assign(heightMap->begin(), heightMap->end());
}

void PlotGenerator::
    prepareHeights(BlockVolume& box, ChunkPos const& /* chunkPos */, bool /* factorInBeardsAndShavers */) {
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

} // namespace plo::core
