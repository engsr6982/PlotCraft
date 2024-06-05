#pragma once
#include "../config/Config.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "mc/math/Vec3.h"
#include "mc/world/level/ChunkPos.h"
#include "plotcraft/config/Config.h"

namespace plotcraft::core {


/*
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{0,-64,0}
    地皮(0,0).maxPos = Vec3{0 + config::cfg.generator.plotWidth,320,0 + config::cfg.generator.plotWidth}

    横向x轴，纵向z轴
    -1,1   0,1   1,1
    -1,0   0,0   1,0
    -1,-1  0,-1  1,-1
 */


class PlotPos {
public:
    int  x, z;   // 地皮坐标
    Vec3 minPos; // 地皮小端坐标
    Vec3 maxPos; // 地皮大端坐标

    // 使用地皮坐标构造
    PlotPos(int x, int z) : x(x), z(z) {
        auto& cfg = config::cfg.generator;
        minPos    = Vec3{x * (cfg.plotWidth + cfg.roadWidth), -64, z * (cfg.plotWidth + cfg.roadWidth)};
        maxPos    = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
    }

    // 使用Vec3构造
    PlotPos(const Vec3& vec3) {
        auto& cfg = config::cfg.generator;
        x         = static_cast<int>(vec3.x) / (cfg.plotWidth + cfg.roadWidth);
        z         = static_cast<int>(vec3.z) / (cfg.plotWidth + cfg.roadWidth);
        minPos    = Vec3{x * (cfg.plotWidth + cfg.roadWidth), -64, z * (cfg.plotWidth + cfg.roadWidth)};
        maxPos    = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};

        // 检查是否在道路上
        int localX = static_cast<int>(vec3.x) % (cfg.plotWidth + cfg.roadWidth);
        int localZ = static_cast<int>(vec3.z) % (cfg.plotWidth + cfg.roadWidth);
        if (localX >= cfg.plotWidth || localZ >= cfg.plotWidth) {
            minPos = Vec3{0, 0, 0};
            maxPos = Vec3{0, 0, 0};
        }
    }

    // 使用ChunkPos构造
    PlotPos(const ChunkPos& chunkPos) {
        auto& cfg = config::cfg.generator;
        x         = chunkPos.x * 16 / (cfg.plotWidth + cfg.roadWidth);
        z         = chunkPos.z * 16 / (cfg.plotWidth + cfg.roadWidth);
        minPos    = Vec3{x * (cfg.plotWidth + cfg.roadWidth), -64, z * (cfg.plotWidth + cfg.roadWidth)};
        maxPos    = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
    }

    Vec3   getMin() { return minPos; }
    Vec3   getMax() { return maxPos; }
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