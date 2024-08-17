#ifdef OVERWORLD
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/OverworldDimension.h"
#include "mc/world/level/levelgen/structure/StructureSetRegistry.h"


#if defined(TEST)
#include "CustomGenerator.h"
#else
#include "PlotGenerator.h"
#endif


LL_AUTO_TYPE_INSTANCE_HOOK(
    OverworldDimensionCreateGeneratorHook,
    ll::memory::HookPriority::Normal,
    OverworldDimension,
    "?createGenerator@OverworldDimension@@UEAA?AV?$unique_ptr@VWorldGenerator@@U?$default_delete@VWorldGenerator@@@std@"
    "@@std@@AEBVStructureSetRegistry@worldgen@br@@@Z",
    std::unique_ptr<WorldGenerator>,
    br::worldgen::StructureSetRegistry const& 
) {
    std::unique_ptr<WorldGenerator> worldGenerator;
    mSeaLevel       = -61;
    auto  seed      = getLevel().getSeed();
    auto& levelData = getLevel().getLevelData();


#if defined(TEST)
    worldGenerator =
        std::make_unique<plo::core::CustomGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());
#else
    worldGenerator = std::make_unique<plo::core::PlotGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());
#endif


    worldGenerator->init();
    return std::move(worldGenerator);
}

#endif // OVERWORLD