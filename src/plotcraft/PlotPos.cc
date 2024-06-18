#include "plotcraft/PlotPos.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "plotcraft/Config.h"
#include <cmath>


namespace plo {

/*
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{0,-64,0}
    地皮(0,0).maxPos = Vec3{0 + config::cfg.generator.plotWidth -1 ,320,0 + config::cfg.generator.plotWidth -1}

    横向x轴，纵向z轴
    -1,1   0,1   1,1
    -1,0   0,0   1,0
    -1,-1  0,-1  1,-1
 */

PlotPos::PlotPos() : x(0), z(0), mIsValid(false) {
    minPos = Vec3{0, 0, 0};
    maxPos = Vec3{0, 0, 0};
}

PlotPos::PlotPos(int x, int z) : x(x), z(z) {
    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotWidth + cfg.roadWidth;
    minPos          = Vec3{x * totalSize, cfg.generatorY, z * totalSize};
    maxPos          = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
}

PlotPos::PlotPos(const Vec3& vec3) {
    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotWidth + cfg.roadWidth;
    int   gridX     = std::floor(vec3.x / totalSize);
    int   gridZ     = std::floor(vec3.z / totalSize);
    int   localX    = static_cast<int>(std::floor(vec3.x)) % totalSize;
    int   localZ    = static_cast<int>(std::floor(vec3.z)) % totalSize;

    if (localX < 0) localX += totalSize;
    if (localZ < 0) localZ += totalSize;

    if (localX >= cfg.plotWidth || localZ >= cfg.plotWidth) {
        // Point is on the road
        minPos   = Vec3{0, 0, 0};
        maxPos   = Vec3{0, 0, 0};
        x        = 0;
        z        = 0;
        mIsValid = false; // 无效的地皮点
    } else {
        x      = gridX;
        z      = gridZ;
        minPos = Vec3{x * totalSize, cfg.generatorY, z * totalSize};
        maxPos = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
    }
}

bool PlotPos::isValid() { return mIsValid; }

Vec3 PlotPos::getMin() { return minPos; }

Vec3 PlotPos::getMax() { return maxPos; }

string PlotPos::toString() { return fmt::format("({0},{1})", x, z); }

string PlotPos::toDebug() { return fmt::format("{0} | {1} => {2}", toString(), minPos.toString(), maxPos.toString()); }

bool PlotPos::isPosInPlot(const Vec3& vec3) {
    return vec3.x >= minPos.x && vec3.x <= maxPos.x && vec3.z >= minPos.z && vec3.z <= maxPos.z;
}

std::vector<PlotPos> PlotPos::getAdjacentPlots() {
    return {PlotPos(x - 1, z), PlotPos(x + 1, z), PlotPos(x, z - 1), PlotPos(x, z + 1)};
}


bool PlotPos::operator==(const PlotPos& other) const {
    return x == other.x && z == other.z && minPos == other.minPos && maxPos == other.maxPos;
}

bool PlotPos::operator!=(const PlotPos& other) const { return !(*this == other); }


} // namespace plo