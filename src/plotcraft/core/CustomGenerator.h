#pragma once
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"
#include <unordered_map>
#include <vector>


namespace plo::core {


enum class GDirection : int {
    East,  // 东
    South, // 南
    West,  // 西
    North, // 北
    NE,    // 东北
    ES,    // 东南
    SW,    // 西南
    WN     // 西北
};


class CustomGenerator : public FlatWorldGenerator {
public:
    int const mChunkNum = 3; // 一个地皮的区块数量

    std::unordered_map<std::string, Block const*> mBlockCache; // 方块缓存

    std::unordered_map<GDirection, std::vector<Block const*>> mChunkPrototypeBlocks = {
        {GDirection::East,  mPrototypeBlocks},
        {GDirection::South, mPrototypeBlocks},
        {GDirection::West,  mPrototypeBlocks},
        {GDirection::North, mPrototypeBlocks},
        {GDirection::NE,    mPrototypeBlocks},
        {GDirection::ES,    mPrototypeBlocks},
        {GDirection::SW,    mPrototypeBlocks},
        {GDirection::WN,    mPrototypeBlocks},
    }; // 方块原型

    std::unordered_map<GDirection, BlockVolume> mChunkPrototypeVolumes = {
        {GDirection::East,  mPrototype},
        {GDirection::South, mPrototype},
        {GDirection::West,  mPrototype},
        {GDirection::North, mPrototype},
        {GDirection::NE,    mPrototype},
        {GDirection::ES,    mPrototype},
        {GDirection::SW,    mPrototype},
        {GDirection::WN,    mPrototype},
    }; // 区块原型

    CustomGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(class LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};

} // namespace plo::core
