#ifdef OVERWORLD
#include "DefaultGenerator.h"
#include "TemplateGenerator.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/OverworldDimension.h"
#include "mc/world/level/levelgen/WorldGenerator.h"
#include "mc\world\level\biome\source\FixedBiomeSource.h"
#include "mc\world\level\levelgen\structure\registry\StructureSetRegistry.h"
#include "mc\world\level\storage\LevelData.h"
#include "plotcraft/Config.h"


LL_AUTO_TYPE_INSTANCE_HOOK(
    OverworldDimensionCreateGeneratorHook,
    ll::memory::HookPriority::Normal,
    OverworldDimension,
    &OverworldDimension::$createGenerator,
    std::unique_ptr<WorldGenerator>,
    br::worldgen::StructureSetRegistry const& /* structureSetRegistry */
) {
    std::unique_ptr<WorldGenerator> worldGenerator;
    mSeaLevel                        = -61;
    auto        seed                 = getLevel().getSeed();
    auto&       levelData            = getLevel().getLevelData();
    auto const& enerationOptionsJSON = levelData.getFlatWorldGeneratorOptions();

    if (plot::Config::cfg.generator.type == plot::PlotGeneratorType::Default) {
        worldGenerator = std::make_unique<plot::core::DefaultGenerator>(*this, seed, enerationOptionsJSON);
    } else {
        worldGenerator = std::make_unique<plot::core::TemplateGenerator>(*this, seed, enerationOptionsJSON);
    }

    // worldGenerator->init(); // removed ?
    return std::move(worldGenerator);
}

#endif // OVERWORLD