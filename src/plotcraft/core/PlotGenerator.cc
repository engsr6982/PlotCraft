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
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/block/utils/BedrockBlockNames.h"
#include "mc/world/level/block/utils/VanillaBlockTypeIds.h"
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

    auto& gen = config::cfg.generator;


    // 初始化方块指针
    mBlock_Air     = &BlockTypeRegistry::getDefaultBlockState("minecraft:air");
    mBlock_Dirt    = &BlockTypeRegistry::getDefaultBlockState("minecraft:dirt");
    mBlock_Grass   = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
    mBlock_Bedrock = &BlockTypeRegistry::getDefaultBlockState("minecraft:bedrock");
    mBlock_Road    = &BlockTypeRegistry::getDefaultBlockState(gen.roadBlock.c_str());
    mBlock_Fill    = &BlockTypeRegistry::getDefaultBlockState(gen.fillBlock.c_str());
    mBlock_Border  = &BlockTypeRegistry::getDefaultBlockState(gen.borderBlock.c_str());
    // 检查用户输入
    if (!mBlock_Road || !mBlock_Fill || !mBlock_Border) {
        throw std::runtime_error("Invalid block type");
    }

    mTemplateSubChunk_0 = TemplateSubChunk(4096, mBlock_Dirt);
    mTemplateSubChunk_1 = TemplateSubChunk(4096, mBlock_Dirt);

    // 设置基岩
    // int next = 0;
    // for (int i = 0; i < 16; i++) {
    //     mTemplateSubChunk_0[next]  = mBlock_Bedrock;
    //     next                      += 16; // 下一个方块的位置
    // }

    mGeneratorY = -60; // 地面高度
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

    auto blockSource = &mDimension->getBlockSourceFromMainChunkSource();
    // auto minHeight   = mDimension->getMinHeight();

    // 生成基础地形
    // buffer_span_mut<Block const*> buffer;
    // buffer.mBegin = &*mTemplateSubChunk_0.begin();
    // buffer.mEnd   = &*mTemplateSubChunk_0.end();
    // levelchunk.setBlockVolume(BlockVolume{buffer, 16, 16, 16, *mBlock_Air, minHeight}, 0);
    // 模板2
    // buffer_span_mut<Block const*> buffer2;
    // buffer2.mBegin = &*mTemplateSubChunk_1.begin();
    // buffer2.mEnd   = &*mTemplateSubChunk_1.end();
    // levelchunk.setBlockVolume(BlockVolume{buffer2, 16, 16, 16, *mBlock_Air, minHeight}, 16);

    levelchunk.setBlockVolume(mPrototype, 0);

    // 生成/计算
    auto& chunkPos = levelchunk.getPosition();

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
                    .setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY, z), -64}, *mBlock_Road, blockSource, nullptr);
                //      北和西（靠近0,0,0）         南和东（地皮对角）
            } else if (gridX == 0 || gridZ == 0 || gridX == gen.plotWidth - 1 || gridZ == gen.plotWidth - 1) {
                // 边框
                levelchunk.setBlock(
                    ChunkBlockPos{BlockPos(x, mGeneratorY + 1, z), -64},
                    *mBlock_Border,
                    blockSource,
                    nullptr
                );
            } else {
                // 地皮内部
                levelchunk
                    .setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY, z), -64}, *mBlock_Fill, blockSource, nullptr);
            }
        }
    }

    levelchunk.recomputeHeightMap(0); // 重新计算高度图
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}

} // namespace plo::core
#endif // GEN_1