#pragma once
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"
#include <memory>
#include <vector>

class ChunkViewSource;
class LevelChunk;
class ChunkPos;

namespace plo::core {

using TemplateSubChunkVector = std::vector<Block const*>;

class DefaultGenerator : public FlatWorldGenerator {
public:
    Block const* mBlock_Dirt;    // 初始方块：泥土
    Block const* mBlock_Bedrock; // 初始方块：基岩

    Block const* mBlock_Road;   // 道路方块
    Block const* mBlock_Border; // 边框方块
    Block const* mBlock_Fill;   // 填充方块


    int mSubChunkNum; // 子区块数量
    int mGeneratorY;  // 地皮生成层

    TemplateSubChunkVector                    mVector_Bedrock1_Dirt15; // 模板：1层基岩 + 15层泥土
    TemplateSubChunkVector                    mVector_Dirt16;          // 模板：16层泥土
    TemplateSubChunkVector                    mVector_Dirt15_Grass1;   // 模板：15层泥土 + 1层草
    std::vector<std::unique_ptr<BlockVolume>> mTemplateSubChunks;      // 子区块地形模板


    DefaultGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);
    void loadChunk(LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};

} // namespace plo::core
