#ifdef OVERWORLD
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/OverworldDimension.h"
#include "mc/world/level/levelgen/structure/StructureFeatureRegistry.h"
#include "mc/world/level/levelgen/structure/StructureSetRegistry.h"
#include "mc/world/level/levelgen/structure/VillageFeature.h"
#include "PlotGenerator.h"
#include "plotcraft/core/Utils.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    OverworldDimensionCreateGeneratorHook,
    ll::memory::HookPriority::Normal,
    OverworldDimension,
    "?createGenerator@OverworldDimension@@UEAA?AV?$unique_ptr@VWorldGenerator@@U?$default_delete@VWorldGenerator@@@std@"
    "@@std@@AEBVStructureSetRegistry@worldgen@br@@@Z",
    std::unique_ptr<WorldGenerator>,
    br::worldgen::StructureSetRegistry const& 
) {
    mSeaLevel = -61;

    std::unique_ptr<WorldGenerator> worldGenerator;
    auto                            seed      = getLevel().getSeed();
    auto&                           levelData = getLevel().getLevelData();

    worldGenerator = std::make_unique<plo::core::PlotGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());

    worldGenerator->init();
    return std::move(worldGenerator);
}

#endif // OVERWORLD