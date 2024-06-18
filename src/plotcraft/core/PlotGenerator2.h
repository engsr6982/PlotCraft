#ifdef GEN_2
#pragma once

#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"
#include <vector>

namespace plo::core {

class PlotGenerator : public FlatWorldGenerator {
public:
    int                       chunk_n    = 4;                // n * n的区块
    std::vector<Block const*> east_side  = mPrototypeBlocks; // 东侧的方块
    std::vector<Block const*> south_side = mPrototypeBlocks; // 南侧的方块
    std::vector<Block const*> west_side  = mPrototypeBlocks; // 西侧的方块
    std::vector<Block const*> north_side = mPrototypeBlocks; // 北侧的方块
    std::vector<Block const*> n_e_angle  = mPrototypeBlocks; // 东北角的方块
    std::vector<Block const*> e_s_angle  = mPrototypeBlocks; // 东南角的方块
    std::vector<Block const*> s_w_angle  = mPrototypeBlocks; // 西南角的方块
    std::vector<Block const*> w_n_angle  = mPrototypeBlocks; // 西北角的方块
    std::vector<Block const*> backup_    = mPrototypeBlocks; // 备份方块

    PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(class LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};

} // namespace plo::core
#endif // GEN_2