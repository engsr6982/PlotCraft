#include "plotcraft/PlotPos.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "plotcraft/Config.h"
#include <cmath>


namespace plo {

/*
    # 生成器1:
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{0,-64,0}
    地皮(0,0).maxPos = Vec3{0 + config::cfg.generator.plotWidth -1 ,320,0 + config::cfg.generator.plotWidth -1}

    # 生成器2:
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{1,-64,1}
    地皮(0,0).maxPos = Vec3{chunk_n * 16 - 2 ,320, chunk_n * 16 - 2} // chunk_n为区块数量,-2为2格固定道路

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
#ifdef GEN_1
    // Generator 1
    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotWidth + cfg.roadWidth;
    minPos          = Vec3{x * totalSize, -64, z * totalSize};
    maxPos          = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
#endif

#ifdef GEN_2
    // Generator 2
    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotChunkSize * 16; // 地皮大小，含道路宽度 (3*16=48)
    minPos          = Vec3{x * totalSize + 1, -64, z * totalSize + 1};
    maxPos          = Vec3{minPos.x + totalSize, 320, minPos.z + totalSize};
#endif
}

PlotPos::PlotPos(const Vec3& vec3) {
#ifdef GEN_1
    // Generator 1
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
        minPos = Vec3{x * totalSize, -64, z * totalSize};
        maxPos = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
    }
#endif

#ifdef GEN_2
    // Generator 2
    auto& cfg       = config::cfg.generator;
    int   plotWidth = (cfg.plotChunkSize * 16) - 2; // 地皮大小 (3*16=48)
    int   totalSize = plotWidth + 2;                // 地皮大小 + 道路宽度
    int   gridX     = std::floor(vec3.x / totalSize);
    int   gridZ     = std::floor(vec3.z / totalSize);
    int   localX    = static_cast<int>(std::floor(vec3.x)) % totalSize;
    int   localZ    = static_cast<int>(std::floor(vec3.z)) % totalSize;

    if (localX < 1) localX += totalSize;
    if (localZ < 1) localZ += totalSize;

    if (localX >= plotWidth || localZ >= plotWidth) {
        // Point is on the road
        minPos   = Vec3{0, 0, 0};
        maxPos   = Vec3{0, 0, 0};
        x        = 0;
        z        = 0;
        mIsValid = false; // 无效的地皮点
    } else {
        x      = gridX;
        z      = gridZ;
        minPos = Vec3{x * totalSize + 1, -64, z * totalSize + 1};
        maxPos = Vec3{minPos.x + plotWidth, 320, minPos.z + plotWidth};
    }
#endif
}

bool PlotPos::isValid() { return mIsValid; }

Vec3 PlotPos::getMin() { return minPos; }

Vec3 PlotPos::getMax() { return maxPos; }

string PlotPos::toString() { return fmt::format("({0},{1})", x, z); }
string PlotPos::getPlotID() { return toString(); }

string PlotPos::toDebug() { return fmt::format("{0} | {1} => {2}", toString(), minPos.toString(), maxPos.toString()); }

bool PlotPos::isPosInPlot(const Vec3& vec3) {
    return vec3.x >= minPos.x && vec3.x <= maxPos.x && vec3.z >= minPos.z && vec3.z <= maxPos.z;
}

std::vector<PlotPos> PlotPos::getAdjacentPlots() {
    return {PlotPos(x - 1, z), PlotPos(x + 1, z), PlotPos(x, z - 1), PlotPos(x, z + 1)};
}


bool PlotPos::isPosOnBorder(const Vec3& vec3) {
    if (vec3.y < -64 || vec3.y > 320) {
        return false;
    }

    tryFixMinAndMaxPos();

    //  X 和 Z 坐标是否在边框上
    bool onXBorder = (vec3.x == minPos.x || vec3.x == maxPos.x);
    bool onZBorder = (vec3.z == minPos.z || vec3.z == maxPos.z);

    //  X 和 Z 坐标是否在地皮范围内
    bool withinXRange = (vec3.x >= minPos.x && vec3.x <= maxPos.x);
    bool withinZRange = (vec3.z >= minPos.z && vec3.z <= maxPos.z);

    // 判断是否在边框上
    return (onXBorder && withinXRange) || (onZBorder && withinZRange);
}


void PlotPos::tryFixMinAndMaxPos() {
    if (minPos.x > maxPos.x) std::swap(minPos.x, maxPos.x);
    if (minPos.y > maxPos.y) std::swap(minPos.y, maxPos.y);
    if (minPos.z > maxPos.z) std::swap(minPos.z, maxPos.z);
}


bool PlotPos::operator!=(const PlotPos& other) const { return !(*this == other); }
bool PlotPos::operator==(const PlotPos& other) const {
    return x == other.x && z == other.z && minPos == other.minPos && maxPos == other.maxPos;
}

} // namespace plo