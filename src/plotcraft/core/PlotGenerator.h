#ifdef GEN_1
#pragma once

#include "mc/deps/core/data/DividedPos2d.h"
#include "mc/deps/core/utility/buffer_span.h"
#include "mc/util/Random.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"


#include <vector>

class ChunkViewSource;
class LevelChunk;
class ChunkPos;

namespace plo::core {

using TemplateSubChunk = std::vector<Block const*>;

class PlotGenerator : public FlatWorldGenerator {
public:
    int mGeneratorY; // 地皮生成层

    Block const* mBlock_Dirt;    // 初始方块：泥土
    Block const* mBlock_Grass;   // 初始方块：草
    Block const* mBlock_Bedrock; // 初始方块：地基
    Block const* mBlock_Air;     // 初始方块：空气

    Block const* mBlock_Road;   // 道路方块
    Block const* mBlock_Border; // 边框方块
    Block const* mBlock_Fill;   // 填充方块

    TemplateSubChunk mTemplateSubChunk_0; // 地形模板：第0层  [1]: 基岩 [2~16]: 泥土
    TemplateSubChunk mTemplateSubChunk_1; // 地形模板：第1层  [1~15]: 泥土 [16]: 草

    PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};

} // namespace plo::core

#endif // GEN_1