#include "PlotDimension.h"

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/DimensionConversionData.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/LevelSeed64.h"
#include "mc/world/level/chunk/ChunkGeneratorStructureState.h"
#include "mc/world/level/chunk/VanillaLevelChunkUpgrade.h"
#include "mc/world/level/dimension/DimensionBrightnessRamp.h"
#include "mc/world/level/dimension/OverworldBrightnessRamp.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "mc/world/level/levelgen/flat/FlatWorldGenerator.h"
#include "mc/world/level/levelgen/structure/StructureFeatureRegistry.h"
#include "mc/world/level/levelgen/structure/StructureSetRegistry.h"
#include "mc/world/level/levelgen/structure/VillageFeature.h"
#include "more_dimensions/api/dimension/CustomDimensionManager.h"

#ifdef GEN_1
#include "PlotGenerator.h"
#endif

#ifdef GEN_2
#include "PlotGenerator2.h"
#endif


namespace plo::core {

PlotDimension::PlotDimension(std::string const& name, more_dimensions::DimensionFactoryInfo const& info)
: Dimension(info.level, info.dimId, {-64, 320}, info.scheduler, name) {
    // 这里说明下，在DimensionFactoryInfo里面more-dimensions会提供维度id，请不要使用固定维度id，避免id冲突导致维度注册出现异常
    mDefaultBrightness.sky   = Brightness::MAX;
    mSeaLevel                = -61;
    mHasWeather              = true;
    mDimensionBrightnessRamp = std::make_unique<OverworldBrightnessRamp>();
    mDimensionBrightnessRamp->buildBrightnessRamp();
}

CompoundTag PlotDimension::generateNewData() { return {}; }

std::unique_ptr<WorldGenerator>
PlotDimension::createGenerator(br::worldgen::StructureSetRegistry const& /* structureSetRegistry */) {
    auto  seed      = getLevel().getSeed();
    auto& levelData = getLevel().getLevelData();

    // 实例化 地皮生成器
    auto worldGenerator =
        std::make_unique<plo::core::PlotGenerator>(*this, seed, levelData.getFlatWorldGeneratorOptions());

    worldGenerator->init(); // 必须调用，初始化生成器

    return std::move(worldGenerator);
}

void PlotDimension::upgradeLevelChunk(ChunkSource& cs, LevelChunk& lc, LevelChunk& generatedChunk) {
    auto blockSource = BlockSource(getLevel(), *this, cs, false, true, false);
    VanillaLevelChunkUpgrade::_upgradeLevelChunkViaMetaData(lc, generatedChunk, blockSource);
    VanillaLevelChunkUpgrade::_upgradeLevelChunkLegacy(lc, blockSource);
}

void PlotDimension::fixWallChunk(ChunkSource& cs, LevelChunk& lc) {
    auto blockSource = BlockSource(getLevel(), *this, cs, false, true, false);
    VanillaLevelChunkUpgrade::fixWallChunk(lc, blockSource);
}

bool PlotDimension::levelChunkNeedsUpgrade(LevelChunk const& lc) const {
    return VanillaLevelChunkUpgrade::levelChunkNeedsUpgrade(lc);
}
void PlotDimension::_upgradeOldLimboEntity(CompoundTag& tag, ::LimboEntitiesVersion vers) {
    auto isTemplate = getLevel().getLevelData().isFromWorldTemplate();
    return VanillaLevelChunkUpgrade::upgradeOldLimboEntity(tag, vers, isTemplate);
}

std::unique_ptr<ChunkSource>
PlotDimension::_wrapStorageForVersionCompatibility(std::unique_ptr<ChunkSource> cs, ::StorageVersion /*ver*/) {
    return cs;
}

Vec3 PlotDimension::translatePosAcrossDimension(Vec3 const& fromPos, DimensionType fromId) const {
    Vec3 topos;
    VanillaDimensions::convertPointBetweenDimensions(
        fromPos,
        topos,
        fromId,
        mId,
        getLevel().getDimensionConversionData()
    );
    constexpr auto clampVal = 32000000.0f - 128.0f;

    topos.x = std::clamp(topos.x, -clampVal, clampVal);
    topos.z = std::clamp(topos.z, -clampVal, clampVal);

    return topos;
}

short PlotDimension::getCloudHeight() const { return 192; }

bool PlotDimension::hasPrecipitationFog() const { return true; }

} // namespace plo::core
