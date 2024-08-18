#include "TemplateGenerator.h"
#include "TemplateManager.h"
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
#include "mc/world/level/block/utils/BedrockBlockNames.h"
#include "mc/world/level/block/utils/VanillaBlockTypeIds.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/v1/ChunkLocalNoiseCache.h"
#include <cstdio>
#include <memory>
#include <vector>


namespace plo::core {


TemplateGenerator::TemplateGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON)
: FlatWorldGenerator(dimension, seed, generationOptionsJSON) {
    mBiome       = getLevel().getBiomeRegistry().lookupByHash(VanillaBiomeNames::Plains);
    mBiomeSource = std::make_unique<FixedBiomeSource>(*mBiome);

    // clang-format off
    mBlockCache["minecraft:grass_block"] = &BlockTypeRegistry::getDefaultBlockState("minecraft:grass_block");
    mBlockCache["minecraft:dirt"]        = &BlockTypeRegistry::getDefaultBlockState("minecraft:dirt");
    mBlockCache["minecraft:smooth_stone_slab"] = &BlockTypeRegistry::getDefaultBlockState("minecraft:smooth_stone_slab");

    auto height = mPrototype.mHeight; // 16
    for (int i = 0; i < 16; i++) {
        // 南边
        mChunkPrototypeBlocks[GDirection::ES][height * 15 + height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::SW][height * 15 + height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::South][height * 15 + height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::South][height * 14 + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
        // 东边
        mChunkPrototypeBlocks[GDirection::ES][height * 16 * 15 + height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::NE][height * 16 * 15 + height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::East][height * 16 * 15 + height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::East][height * 16 * 14 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
        // 西边
        mChunkPrototypeBlocks[GDirection::SW][height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::WN][height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::West][height * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::West][height * 16 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
        // 北边
        mChunkPrototypeBlocks[GDirection::NE][height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::WN][height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::North][height * 16 * i + 3] = mBlockCache["minecraft:grass_block"];
        mChunkPrototypeBlocks[GDirection::North][height + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];

        // 半砖避免放到路上，最后一次循环
        if (i < 15) {
            // 南
            mChunkPrototypeBlocks[GDirection::ES][height * 14 + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 东
            mChunkPrototypeBlocks[GDirection::ES][height * 16 * 14 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 西
            mChunkPrototypeBlocks[GDirection::SW][height * 16 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 北
            mChunkPrototypeBlocks[GDirection::NE][height + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
        }
        // 第一次循环
        if (i > 0) {
            // 西
            mChunkPrototypeBlocks[GDirection::WN][height * 16 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 北
            mChunkPrototypeBlocks[GDirection::WN][height + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 南
            mChunkPrototypeBlocks[GDirection::SW][height * 14 + height * 16 * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
            // 东
            mChunkPrototypeBlocks[GDirection::NE][height * 16 * 14 + height * i + 4] = mBlockCache["minecraft:smooth_stone_slab"];
        }
    }
    // clang-format on

    // 重新赋值处理
    mChunkPrototypeVolumes[GDirection::East].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::East].begin();
    mChunkPrototypeVolumes[GDirection::East].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::East].end();

    mChunkPrototypeVolumes[GDirection::South].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::South].begin();
    mChunkPrototypeVolumes[GDirection::South].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::South].end();

    mChunkPrototypeVolumes[GDirection::West].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::West].begin();
    mChunkPrototypeVolumes[GDirection::West].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::West].end();

    mChunkPrototypeVolumes[GDirection::North].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::North].begin();
    mChunkPrototypeVolumes[GDirection::North].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::North].end();

    mChunkPrototypeVolumes[GDirection::ES].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::ES].begin();
    mChunkPrototypeVolumes[GDirection::ES].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::ES].end();

    mChunkPrototypeVolumes[GDirection::SW].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::SW].begin();
    mChunkPrototypeVolumes[GDirection::SW].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::SW].end();

    mChunkPrototypeVolumes[GDirection::NE].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::NE].begin();
    mChunkPrototypeVolumes[GDirection::NE].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::NE].end();

    mChunkPrototypeVolumes[GDirection::WN].mBlocks.mBegin = &*mChunkPrototypeBlocks[GDirection::WN].begin();
    mChunkPrototypeVolumes[GDirection::WN].mBlocks.mEnd   = &*mChunkPrototypeBlocks[GDirection::WN].end();
}


void TemplateGenerator::loadChunk(LevelChunk& levelchunk, bool) {
    auto& chunkPos = levelchunk.getPosition();

    // levelchunk.setBlockVolume(TemplateManager::mBlockVolume[TemplateManager::calculateChunkID(chunkPos)], 0);


    int const& n     = mChunkNum;
    auto       pos_x = (chunkPos.x % n + n) % n;
    auto       pos_z = (chunkPos.z % n + n) % n;

    // 控制生成道路
    if (pos_x == 0) {
        if (pos_z == 0) {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::WN], 0); // 西北角
        } else if (pos_z == (n - 1)) {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::SW], 0); // 西南角
        } else {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::West], 0); // 西
        }

    } else if (pos_x == (n - 1)) {
        if (pos_z == 0) {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::NE], 0); // 东北角
        } else if (pos_z == (n - 1)) {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::ES], 0); // 东南角
        } else {
            levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::East], 0); // 东
        }
    } else if (pos_z == 0) {
        levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::North], 0); // 北
    } else if (pos_z == (n - 1)) {
        levelchunk.setBlockVolume(mChunkPrototypeVolumes[GDirection::South], 0); // 南
    } else {
        levelchunk.setBlockVolume(mPrototype, 0); // 中间
    }


    levelchunk.recomputeHeightMap(0);
    mBiomeSource->fillBiomes(levelchunk, ChunkLocalNoiseCache{});
    levelchunk.setSaved();
    levelchunk.changeState(ChunkState::Generating, ChunkState::Generated);
}


} // namespace plo::core
