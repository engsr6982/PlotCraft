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

class PlotGenerator : public FlatWorldGenerator {
public:
    int mGeneratorY; // 地皮生成层

    PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};

} // namespace plo::core

#endif // GEN_1