#include <cstdio>
#ifdef GEN_2
#include "PlotGenerator2.h"
#include "plotcraft/Config.h"

#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/block/utils/BedrockBlockNames.h"
#include "mc/world/level/block/utils/VanillaBlockTypeIds.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"

namespace plo::core {

PlotGenerator::PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    auto& gen = config::cfg.generator;
    chunk_n   = gen.plotChunkSize;

    mBlock_GrassPath      = &BlockTypeRegistry::getDefaultBlockState(gen.roadBlock.c_str());
    mBlock_StoneBlockSlab = &BlockTypeRegistry::getDefaultBlockState(gen.borderBlock.c_str());

    auto height = mPrototype.mHeight; // 16
    for (int i = 0; i < 16; i++) {
        // 一般开始的是距离第一个方块的差值，然后是会乘
        // i的间隔方块，然后是与另外一条路或者半砖方块的间隔，最后面的是第几层 南边
        e_s_angle[height * 15 + height * 16 * i + 3]  = mBlock_GrassPath;
        s_w_angle[height * 15 + height * 16 * i + 3]  = mBlock_GrassPath;
        south_side[height * 15 + height * 16 * i + 3] = mBlock_GrassPath;
        south_side[height * 14 + height * 16 * i + 4] = mBlock_StoneBlockSlab;
        // 东边
        e_s_angle[height * 16 * 15 + height * i + 3] = mBlock_GrassPath;
        n_e_angle[height * 16 * 15 + height * i + 3] = mBlock_GrassPath;
        east_side[height * 16 * 15 + height * i + 3] = mBlock_GrassPath;
        east_side[height * 16 * 14 + height * i + 4] = mBlock_StoneBlockSlab;
        // 西边
        s_w_angle[height * i + 3]               = mBlock_GrassPath;
        w_n_angle[height * i + 3]               = mBlock_GrassPath;
        west_side[height * i + 3]               = mBlock_GrassPath;
        west_side[height * 16 + height * i + 4] = mBlock_StoneBlockSlab;
        // 北边
        n_e_angle[height * 16 * i + 3]           = mBlock_GrassPath;
        w_n_angle[height * 16 * i + 3]           = mBlock_GrassPath;
        north_side[height * 16 * i + 3]          = mBlock_GrassPath;
        north_side[height + height * 16 * i + 4] = mBlock_StoneBlockSlab;

        // 半砖避免放到路上，最后一次循环
        if (i < 15) {
            // 南
            e_s_angle[height * 14 + height * 16 * i + 4] = mBlock_StoneBlockSlab;
            // 东
            e_s_angle[height * 16 * 14 + height * i + 4] = mBlock_StoneBlockSlab;
            // 西
            s_w_angle[height * 16 + height * i + 4] = mBlock_StoneBlockSlab;
            // 北
            n_e_angle[height + height * 16 * i + 4] = mBlock_StoneBlockSlab;
        }
        // 第一次循环
        if (i > 0) {
            // 西
            w_n_angle[height * 16 + height * i + 4] = mBlock_StoneBlockSlab;
            // 北
            w_n_angle[height + height * 16 * i + 4] = mBlock_StoneBlockSlab;
            // 南
            s_w_angle[height * 14 + height * 16 * i + 4] = mBlock_StoneBlockSlab;
            // 东
            n_e_angle[height * 16 * 14 + height * i + 4] = mBlock_StoneBlockSlab;
        }
    }
    // 重新赋值处理
    mVol_East_Side.mBlocks.mBegin = &*east_side.begin();
    mVol_East_Side.mBlocks.mEnd   = &*east_side.end();

    mVol_South_Side.mBlocks.mBegin = &*south_side.begin();
    mVol_South_Side.mBlocks.mEnd   = &*south_side.end();

    mVol_West_Side.mBlocks.mBegin = &*west_side.begin();
    mVol_West_Side.mBlocks.mEnd   = &*west_side.end();

    mVol_North_Side.mBlocks.mBegin = &*north_side.begin();
    mVol_North_Side.mBlocks.mEnd   = &*north_side.end();

    mVol_ES_Angle.mBlocks.mBegin = &*e_s_angle.begin();
    mVol_ES_Angle.mBlocks.mEnd   = &*e_s_angle.end();

    mVol_SW_Angle.mBlocks.mBegin = &*s_w_angle.begin();
    mVol_SW_Angle.mBlocks.mEnd   = &*s_w_angle.end();

    mVol_NE_Angle.mBlocks.mBegin = &*n_e_angle.begin();
    mVol_NE_Angle.mBlocks.mEnd   = &*n_e_angle.end();

    mVol_WN_Angle.mBlocks.mBegin = &*w_n_angle.begin();
    mVol_WN_Angle.mBlocks.mEnd   = &*w_n_angle.end();
}


void PlotGenerator::loadChunk(LevelChunk& levelchunk, bool) {
    auto chunkPos = levelchunk.getPosition();

    int  n     = chunk_n;
    auto pos_x = (chunkPos.x % n + n) % n;
    auto pos_z = (chunkPos.z % n + n) % n;

    // 控制生成道路
    if (pos_x == 0) {
        if (pos_z == 0) {
            levelchunk.setBlockVolume(mVol_WN_Angle, 0); // 西北角
        } else if (pos_z == (n - 1)) {
            levelchunk.setBlockVolume(mVol_SW_Angle, 0); // 西南角
        } else {
            levelchunk.setBlockVolume(mVol_West_Side, 0); // 西
        }

    } else if (pos_x == (n - 1)) {
        if (pos_z == 0) {
            levelchunk.setBlockVolume(mVol_NE_Angle, 0); // 东北角
        } else if (pos_z == (n - 1)) {
            levelchunk.setBlockVolume(mVol_ES_Angle, 0); // 东南角
        } else {
            levelchunk.setBlockVolume(mVol_East_Side, 0); // 东
        }
    } else if (pos_z == 0) {
        levelchunk.setBlockVolume(mVol_North_Side, 0); // 北
    } else if (pos_z == (n - 1)) {
        levelchunk.setBlockVolume(mVol_South_Side, 0); // 南
    } else {
        levelchunk.setBlockVolume(mPrototype, 0); // 中间
    }


    levelchunk.recomputeHeightMap(0);
    // mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);
    // ChunkLocalNoiseCache chunkLocalNoiseCache;
    // mBiomeSource->fillBiomes(levelchunk, chunkLocalNoiseCache);
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}


} // namespace plo::core

#endif // GEN_2