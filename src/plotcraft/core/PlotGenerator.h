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

// 我们直接继承原版超平坦这个类来写会方便很多
class PlotGenerator : public FlatWorldGenerator {
public:
    Random random; // 这个是BDS生成随机数有关的类

    // 后面的generationOptionsJSON虽然用不上，但FlatWorldGenerator的实例化需要
    PlotGenerator(Dimension& dimension, uint seed, Json::Value const& generationOptionsJSON);

    // 这里是处理结构放置相关的，包括地物，结构，地形
    bool postProcess(ChunkViewSource& neighborhood);

    // 这里是初始处理新的单区块的方块生成相关的，比如一些大量的方块（石头，泥土）
    void loadChunk(LevelChunk& levelchunk, bool forceImmediateReplacementDataLoad);

    // 判断某个点在哪个结构范围里
    StructureFeatureType findStructureFeatureTypeAt(BlockPos const&);

    // 判断某个点是否在某个结构范围里
    bool isStructureFeatureTypeAt(BlockPos const&, ::StructureFeatureType) const;

    // 这里是获取某个坐标的最高方块
    std::optional<short> getPreliminarySurfaceLevel(DividedPos2d<4> worldPos) const;

    // 如意，以一个坐标，在一定范围内查找某个类型的结构
    bool
    findNearestStructureFeature(::StructureFeatureType, BlockPos const&, BlockPos&, bool, std::optional<HashedString>);

    // 无需在意，照写就行
    void garbageCollectBlueprints(buffer_span<ChunkPos>);

    // 处理地形
    void prepareHeights(BlockVolume& box, ChunkPos const& chunkPos, bool factorInBeardsAndShavers);

    // 与prepareHeights一样，不过与之不同的是，还会计算单区块内的高度
    void prepareAndComputeHeights(
        BlockVolume&        box,
        ChunkPos const&     chunkPos,
        std::vector<short>& ZXheights,
        bool                factorInBeardsAndShavers,
        int                 skipTopN
    );

    // 可选，可以不写
    BlockPos findSpawnPosition() const { return {0, 16, 0}; };
};

} // namespace plo::core

#endif // GEN_1