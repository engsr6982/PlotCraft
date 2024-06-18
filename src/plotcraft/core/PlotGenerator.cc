#ifdef GEN_1
#include "PlotGenerator.h"
#include "plotcraft/Config.h"

#include "fmt/format.h"
#include "mc/deps/core/data/DividedPos2d.h"
#include "mc/deps/core/utility/buffer_span_mut.h"
#include "mc/network/packet/UpdateSubChunkBlocksPacket.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/VanillaBiomeNames.h"
#include "mc/world/level/biome/registry/BiomeRegistry.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/PostprocessingManager.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include <cstdio>
#include <sstream>


namespace plo::core {

PlotGenerator::PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains);
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);

    // 动态确定生成层
    int y = -64;
    for (auto bl : mPrototypeBlocks) {
        if (bl->getTypeName() == "minecraft:air") {
            mGeneratorY = y;
            break;
        }
        y++;
    }
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


#ifdef DEBUG
    static bool log = false;
    if (!log) {
        auto               a = mPrototype.mBlocks.begin();
        auto               b = mPrototype.mBlocks.end();
        std::ostringstream oss;
        oss << "mPrototype.mWidth: " << mPrototype.mWidth << std::endl;
        oss << "mPrototype.mHeight: " << mPrototype.mHeight << std::endl;
        oss << "mPrototype.mDepth: " << mPrototype.mDepth << std::endl;
        oss << "mPrototype.mDimensionBottom: " << mPrototype.mDimensionBottom << std::endl;
        if (mPrototype.mInitBlock != nullptr)
            oss << "mPrototype.mInitBlock: " << mPrototype.mInitBlock->getName().c_str() << std::endl;
        if (a != nullptr)
            if ((*a) != nullptr) oss << "mPrototype.mBlocks.mBegin: " << (*a)->getName().c_str() << std::endl;
        if (b != nullptr)
            if ((*b) != nullptr) oss << "mPrototype.mBlocks.mEnd: " << (*b)->getName().c_str() << std::endl;
        log = true;
        std::cout << oss.str() << std::endl;
    }
#endif


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
                levelchunk.setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY, z), -64}, roadBlock, blockSource, nullptr);
            } else if (gridX == gen.roadWidth - 3 || gridZ == gen.roadWidth - 3 || gridX == gen.plotWidth - 1 || gridZ == gen.plotWidth - 1) {
                // 边框
                levelchunk
                    .setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY + 1, z), -64}, borderBlock, blockSource, nullptr);
            } else {
                // 地皮内部
                levelchunk.setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY, z), -64}, fillBlock, blockSource, nullptr);
            }
        }
    }

    levelchunk.recomputeHeightMap(0); // 重新计算高度图
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}

} // namespace plo::core
#endif // GEN_1