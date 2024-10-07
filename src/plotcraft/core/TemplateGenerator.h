#pragma once
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"


namespace plot::core {


class TemplateGenerator : public FlatWorldGenerator {
public:
    TemplateGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(class LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};


} // namespace plot::core
