#pragma once
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"


namespace plo::core {


class TemplateGenerator : public FlatWorldGenerator {
public:
    TemplateGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    void loadChunk(class LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);
};


} // namespace plo::core
