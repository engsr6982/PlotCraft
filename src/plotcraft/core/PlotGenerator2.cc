#include <cstdio>
#ifdef GEN_2
#include "PlotGenerator2.h"

#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/block/utils/BedrockBlockNames.h"
#include "mc/world/level/block/utils/VanillaBlockTypeIds.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"

namespace plo::core {

PlotGenerator::PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {

    // int _i = 0;
    // for (auto* block : mPrototypeBlocks) {
    //     std::cout << "[" + std::to_string(_i++) + "] " + block->getName().getString() << std::endl;
    // }
    // 此for循环打印得出：
    // mPrototypeBlocks 穷举了 16*16*16 区域的方块
    // 0    -> bedrock
    // 1~2  -> dirt
    // 3    -> grass_block
    // 4~15 -> air
    // ....(重复循环以上)
    // 此数组长度 4095 即 16*16*16

    // buffer_span_mut.begin => minecraft:bedrock
    // buffer_span_mut.end => nullptr

    // BlockVolume.initBlock 用于 setBlockVolume 不是 mPrototype 时，用这个函数来填充方块
    // BlockVolume 的 height、width、depth 都是 16，即正好和 mPrototypeBlocks 的大小一致

    // 由此得出：
    // 1. Mc是靠buffer_span_mut的begin和end的Block**访问mPrototypeBlocks确定一个y轴生成的方块

    auto height = mPrototype.mHeight; // 16
    for (int i = 0; i < 16; i++) {
        // 一般开始的是距离第一个方块的差值，然后是会乘
        // i的间隔方块，然后是与另外一条路或者半砖方块的间隔，最后面的是第几层 南边
        e_s_angle[height * 15 + height * 16 * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        s_w_angle[height * 15 + height * 16 * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        south_side[height * 15 + height * 16 * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
        south_side[height * 14 + height * 16 * i + 4] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
        // 东边
        e_s_angle[height * 16 * 15 + height * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        n_e_angle[height * 16 * 15 + height * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        east_side[height * 16 * 15 + height * i + 3] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
        east_side[height * 16 * 14 + height * i + 4] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
        // 西边
        s_w_angle[height * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        w_n_angle[height * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        west_side[height * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
        west_side[height * 16 + height * i + 4] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
        // 北边
        n_e_angle[height * 16 * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        w_n_angle[height * 16 * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");

        north_side[height * 16 * i + 3] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
        north_side[height + height * 16 * i + 4] =
            &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");

        // 半砖避免放到路上，最后一次循环
        if (i < 15) {
            // 南
            e_s_angle[height * 14 + height * 16 * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 东
            e_s_angle[height * 16 * 14 + height * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 西
            s_w_angle[height * 16 + height * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 北
            n_e_angle[height + height * 16 * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
        }
        // 第一次循环
        if (i > 0) {
            // 西
            w_n_angle[height * 16 + height * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 北
            w_n_angle[height + height * 16 * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 南
            s_w_angle[height * 14 + height * 16 * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
            // 东
            n_e_angle[height * 16 * 14 + height * i + 4] =
                &BlockTypeRegistry::getDefaultBlockState("minecraft:stone_block_slab");
        }
    }
}

#ifdef DEBUG

#define DLOG(x) printf(x);

#else

#define DLOG(x)

#endif

void PlotGenerator::loadChunk(LevelChunk& levelchunk, bool) {
    auto chunkPos = levelchunk.getPosition();

    int  n         = chunk_n;
    auto pos_x     = (chunkPos.x % n + n) % n;
    auto pos_z     = (chunkPos.z % n + n) % n;
    auto height    = mPrototype.mHeight;
    auto minHeight = mDimension->getMinHeight();

    auto& defaultBlock =
        BlockTypeRegistry::getDefaultBlockState(BedrockBlockNames::Air, true); // 整个世界的默认填充方块

    buffer_span_mut<Block const*> buffer;

    // 控制生成道路
    if (pos_x == 0) {
        if (pos_z == 0) {
            DLOG("w_n_angle");
            buffer.mBegin = &*w_n_angle.begin(); // 处理西北角
            buffer.mEnd   = &*w_n_angle.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        } else if (pos_z == (n - 1)) {
            DLOG("s_w_angle");
            buffer.mBegin = &*s_w_angle.begin(); // 处理西南角
            buffer.mEnd   = &*s_w_angle.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        } else {
            DLOG("west_side");
            buffer.mBegin = &*west_side.begin(); // 处理西边
            buffer.mEnd   = &*west_side.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        }

    } else if (pos_x == (n - 1)) {
        if (pos_z == 0) {
            DLOG("n_e_angle");
            buffer.mBegin = &*n_e_angle.begin(); // 处理东北角
            buffer.mEnd   = &*n_e_angle.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        } else if (pos_z == (n - 1)) {
            DLOG("e_s_angle");
            buffer.mBegin = &*e_s_angle.begin(); // 处理东南角
            buffer.mEnd   = &*e_s_angle.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        } else {
            DLOG("east_side");
            buffer.mBegin = &*east_side.begin(); // 处理东边
            buffer.mEnd   = &*east_side.end();
            levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
        }
    } else if (pos_z == 0) {
        DLOG("north_side");
        buffer.mBegin = &*north_side.begin(); // 处理北边
        buffer.mEnd   = &*north_side.end();
        levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
    } else if (pos_z == (n - 1)) {
        DLOG("south_side");
        buffer.mBegin = &*south_side.begin(); // 处理南边
        buffer.mEnd   = &*south_side.end();
        levelchunk.setBlockVolume(BlockVolume(buffer, 16, height, 16, defaultBlock, minHeight), 0);
    } else {
        levelchunk.setBlockVolume(mPrototype, 0); // 生成地皮
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