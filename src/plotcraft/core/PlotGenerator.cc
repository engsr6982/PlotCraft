#ifndef OVERWORLD

#include "PlotGenerator.h"
#include "mc/deps/core/utility/buffer_span_mut.h"
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
#include "mc/world/level/block/utils/VanillaBlockTypeIds.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include "plotcraft/Config.h"
#include <cstdio>
#include <memory>
#include <vector>


namespace plo::core {

PlotGenerator::PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains);
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);

    auto& gen = config::cfg.generator;


    // 初始化方块指针
    mBlock_Dirt    = &BlockTypeRegistry::getDefaultBlockState(VanillaBlockTypeIds::Dirt);
    mBlock_Bedrock = &BlockTypeRegistry::getDefaultBlockState(VanillaBlockTypeIds::Bedrock);
    mBlock_Road    = &BlockTypeRegistry::getDefaultBlockState(gen.roadBlock.c_str());
    mBlock_Fill    = &BlockTypeRegistry::getDefaultBlockState(gen.fillBlock.c_str());
    mBlock_Border  = &BlockTypeRegistry::getDefaultBlockState(gen.borderBlock.c_str());
    // 检查用户输入
    if (!mBlock_Road || !mBlock_Fill || !mBlock_Border) {
        throw std::runtime_error("Invalid block type");
    }

    mSubChunkNum = gen.subChunkNum;
    mGeneratorY  = -64 + (mSubChunkNum * 16) - 1;

    // 子区块模板方块
    mVector_Dirt16          = TemplateSubChunkVector(4096, mBlock_Dirt);
    mVector_Bedrock1_Dirt15 = TemplateSubChunkVector(4096, mBlock_Dirt);
    mVector_Dirt15_Grass1   = TemplateSubChunkVector(4096, mBlock_Dirt);


    for (size_t i = 0; i < 256; ++i) {
        mVector_Bedrock1_Dirt15[i * 16]      = mBlock_Bedrock; // 将每个子区块的第一个方块设置为基岩
        mVector_Dirt15_Grass1[(i * 16) + 15] = mBlock_Fill;    // 将每个子区块的第15方块设置为草方块
    }


    // 生成4个模板子区块Volume
    for (int i = 0; i < mSubChunkNum; i++) {
        // 拷贝原有的 BlockVolume 而不使用构造函数（使用构造函数不生效，原因未知）
        auto ptr            = std::make_unique<BlockVolume>(mPrototype);
        ptr->mBlocks.mBegin = &*mVector_Dirt16.begin(); // 指向 mVector_Dirt
        ptr->mBlocks.mEnd   = &*mVector_Dirt16.end();
        mTemplateSubChunks.push_back(std::move(ptr));
    }
    // 确保有基岩和草方块
    auto begin            = mTemplateSubChunks[0].get();
    begin->mBlocks.mBegin = &*mVector_Bedrock1_Dirt15.begin(); // 指向 mVector_Bedrock1_Dirt15
    begin->mBlocks.mEnd   = &*mVector_Bedrock1_Dirt15.end();
    auto end              = mTemplateSubChunks[mSubChunkNum - 1].get();
    end->mBlocks.mEnd     = &*mVector_Dirt15_Grass1.end(); // 指向 mVector_Dirt15_Grass1
    end->mBlocks.mBegin   = &*mVector_Dirt15_Grass1.begin();

    // 设置群系
    mBiomeSource =
        std::make_unique<FixedBiomeSource>(*dimension.getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains));
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

    // 生成模板地形
    int next = 0;
    for (int i = 0; i < mSubChunkNum; i++) {
        levelchunk.setBlockVolume(*mTemplateSubChunks[i].get(), next);
        next += 16;
    }


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
            }
            // else {
            //     // 地皮内部(使用模板地形生成的草方块)
            //     levelchunk
            //         .setBlock(ChunkBlockPos{BlockPos(x, mGeneratorY, z), -64}, *mBlock_Fill, blockSource, nullptr);
            // }
        }
    }

    // try fill biomes
    this->mBiomeSource->fillBiomes(levelchunk, ChunkLocalNoiseCache{});

    levelchunk.recomputeHeightMap(0); // 重新计算高度图
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}

} // namespace plo::core

#endif // OVERWORLD