#include "../config/Config.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "mc/math/Vec3.h"
#include "mc/world/level/ChunkPos.h"


namespace plotcraft::core {


/*
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{0+3,-64,0+3}
    地皮(0,0).maxPos = Vec3{0+3+64,320,0+3+64}
 */


class PlotPos {
public:
    const int x, z;   // 地皮坐标
    Vec3      minPos; // 地皮小端坐标
    Vec3      maxPos; // 地皮大端坐标

    // 构造函数
    PlotPos(int x, int z) : x(x), z(z) {
        auto& gen      = config::cfg.generator;
        int   plotSize = gen.plotWidth + gen.roadWidth;
        minPos         = Vec3{x * plotSize + gen.roadWidth, -64, z * plotSize + gen.roadWidth};
        maxPos         = Vec3{minPos.x + gen.plotWidth, 320, minPos.z + gen.plotWidth};
    }

    PlotPos(const Vec3& vec3)
    : PlotPos(
        vec3.x / (config::cfg.generator.plotWidth + config::cfg.generator.roadWidth),
        vec3.z / (config::cfg.generator.plotWidth + config::cfg.generator.roadWidth)
    ) {}

    PlotPos(const ChunkPos& chunkPos) : PlotPos(chunkPos.x * 16, chunkPos.z * 16) {}

    // 获取地皮的最小坐标
    Vec3 getMin() { return minPos; }

    // 获取地皮的最大坐标
    Vec3 getMax() { return maxPos; }

    string toString() { return fmt::format("({0},{1})", x, z); }
    string toDebug() { return fmt::format("{0} | {1} => {2}", toString(), minPos.toString(), maxPos.toString()); }

    // 检查给定的 Vec3 坐标是否在地皮内
    bool isPosInPlot(const Vec3& vec3) {
        return vec3.x >= minPos.x && vec3.x <= maxPos.x && vec3.z >= minPos.z && vec3.z <= maxPos.z;
    }

    // 获取相邻的地皮
    std::vector<PlotPos> getAdjacentPlots() {
        return {PlotPos(x - 1, z), PlotPos(x + 1, z), PlotPos(x, z - 1), PlotPos(x, z + 1)};
    }
};

} // namespace plotcraft::core