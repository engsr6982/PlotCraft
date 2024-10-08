#ifdef OVERWORLD
#include "DefaultGenerator.h"
#include "TemplateGenerator.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/OverworldDimension.h"
#include "mc/world/level/levelgen/WorldGenerator.h"
#include "mc/world/level/levelgen/structure/StructureSetRegistry.h"
#include "plotcraft/Config.h"


LL_AUTO_TYPE_INSTANCE_HOOK(
    OverworldDimensionCreateGeneratorHook,
    ll::memory::HookPriority::Normal,
    OverworldDimension,
    "?createGenerator@OverworldDimension@@UEAA?AV?$unique_ptr@VWorldGenerator@@U?$default_delete@VWorldGenerator@@@std@"
    "@@std@@AEBVStructureSetRegistry@worldgen@br@@@Z",
    // &OverworldDimension::createGenerator,
    std::unique_ptr<WorldGenerator>,
    br::worldgen::StructureSetRegistry const& 
) {
    std::unique_ptr<WorldGenerator> worldGenerator;
    mSeaLevel       = -61;
    auto  seed      = getLevel().getSeed();
    auto& levelData = getLevel().getLevelData();

    if (plot::Config::cfg.generator.type == plot::Config::PlotGeneratorType::Default) {
        worldGenerator =
            std::make_unique<plot::core::DefaultGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());
    } else {
        worldGenerator =
            std::make_unique<plot::core::TemplateGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());
    }

    worldGenerator->init();
    return std::move(worldGenerator);
}

#endif // OVERWORLD