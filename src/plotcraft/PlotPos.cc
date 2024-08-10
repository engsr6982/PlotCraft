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
    maxPos          = Vec3{minPos.x + totalSize - 1, 320, minPos.z + totalSize - 1};
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
    int   totalSize = cfg.plotChunkSize * 16; // 地皮大小 + 道路宽度 => (3*16=48)
    int   plotWidth = totalSize - 2;          // 地皮大小 (48-2=46)

    int gridX = std::floor(vec3.x / totalSize);
    int gridZ = std::floor(vec3.z / totalSize);
    int localX = static_cast<int>(std::floor(vec3.x)) % totalSize; // 46 % 48 = 46 => (当前坐标和完整大小取模)
    int localZ = static_cast<int>(std::floor(vec3.z)) % totalSize;

    if (localX < 1) localX += totalSize; // 0 + 1 = 1 => border
    if (localZ < 1) localZ += totalSize;

    // 1 > 1     46 > 46
    if (localX > plotWidth || localZ > plotWidth) {
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
        maxPos = Vec3{minPos.x + plotWidth - 1, 320, minPos.z + plotWidth - 1};
    }
#endif
}

int    PlotPos::getSurfaceY() const { return -64 + (config::cfg.generator.subChunkNum * 16); }
Vec3   PlotPos::getSafestPos() const { return Vec3{minPos.x, getSurfaceY() + 1, minPos.z}; }
bool   PlotPos::isValid() const { return mIsValid; }
Vec3   PlotPos::getMin() const { return minPos; } // deprecated
Vec3   PlotPos::getMax() const { return maxPos; } // deprecated
string PlotPos::toString() const { return fmt::format("({0},{1})", x, z); }
string PlotPos::getPlotID() const { return toString(); }


bool PlotPos::isPosInPlot(const Vec3& vec3) const {
    return vec3.x >= minPos.x && vec3.x <= maxPos.x && vec3.z >= minPos.z && vec3.z <= maxPos.z;
}

std::vector<PlotPos> PlotPos::getAdjacentPlots() const {
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

string PlotPos::toDebug() const {
#if !defined(DEBUG)
    return fmt::format("{0} | {1} => {2}", toString(), minPos.toString(), maxPos.toString());
#else
    PlotPos ps(1, 0); // (1,0)
    auto    r = PlotPos::getAdjacentPlotRoad(*this, ps);
    return fmt::format(
        "{0} | {1} => {2}\n相邻: {3}  |  {4} => {5}\n道路: {6} => {7}",
        toString(),
        minPos.toString(),
        maxPos.toString(),
        PlotPos::isAdjacent(*this, ps),
        toString(),
        ps.toString(),
        r.first.toString(),
        r.second.toString()
    );
#endif
}
bool PlotPos::operator!=(PlotPos const& other) const { return !(*this == other); }
bool PlotPos::operator==(PlotPos const& other) const {
    return other.x == x && other.z == z && other.mIsValid == mIsValid && other.minPos == minPos
        && other.maxPos == maxPos;
}


DiagonPos PlotPos::getDiagonPos() const { return DiagonPos{minPos, maxPos}; }
DiagonPos PlotPos::getBorderDiagonPos(Direction direction) const {
    int y = getSurfaceY();
    switch (direction) {
    case Direction::North:
        return DiagonPos{
            Vec3{minPos.x, y, minPos.z}, // 0,0,0
            Vec3{maxPos.x, y, minPos.z}  // 127,0,0
        };
    case Direction::East:
        return DiagonPos{
            Vec3{maxPos.x, y, minPos.z}, // 127,0,0
            Vec3{maxPos.x, y, maxPos.z}  // 127,0,127
        };
    case Direction::South:
        return DiagonPos{
            Vec3{minPos.x, y, maxPos.z}, // 0,0,127
            Vec3{maxPos.z, y, maxPos.z}  // 127,0,127
        };
    case Direction::West:
        return DiagonPos{
            Vec3{minPos.x, y, minPos.z}, // 0,0,0
            Vec3{minPos.x, y, maxPos.z}  // 0,0,127
        };
    }
}


// static
bool PlotPos::isAdjacent(const PlotPos& plot1, const PlotPos& plot2) {
    int dx = std::abs(plot1.x - plot2.x);
    int dz = std::abs(plot1.z - plot2.z);

    // 两个地皮相邻的条件:
    // 1. x坐标相同,z坐标相差1,或者
    // 2. z坐标相同,x坐标相差1
    // 3. 两个地皮都是有效的
    return ((dx == 0 && dz == 1) || (dx == 1 && dz == 0)) && (plot1.mIsValid && plot2.mIsValid);
}
DiagonPos PlotPos::getAdjacentPlotRoad(const PlotPos& plot1, const PlotPos& plot2) {
    if (!isAdjacent(plot1, plot2)) {
        // 如果两个地皮不相邻,返回无效的对角坐标
        return DiagonPos{
            Vec3{0, 0, 0},
            Vec3{0, 0, 0}
        };
    }

    auto& cfg = config::cfg.generator;
    int   y   = plot1.getSurfaceY();

#ifdef GEN_1
    int totalSize = cfg.plotWidth + cfg.roadWidth;

    if (plot1.x == plot2.x) {
        // 地皮在同一列,道路是水平的
        int minZ = std::min(plot1.z, plot2.z) * totalSize + cfg.plotWidth;
        int maxZ = minZ + cfg.roadWidth;
        int x    = plot1.x * totalSize;
        return DiagonPos{
            Vec3{x,                     y, minZ - 1},
            Vec3{x + cfg.plotWidth - 1, y, maxZ    }
        };
    } else {
        // 地皮在同一行,道路是垂直的
        int minX = std::min(plot1.x, plot2.x) * totalSize + cfg.plotWidth;
        int maxX = minX + cfg.roadWidth;
        int z    = plot1.z * totalSize;
        return DiagonPos{
            Vec3{minX - 1, y, z                    },
            Vec3{maxX,     y, z + cfg.plotWidth - 1}
        };
    }
#endif

#ifdef GEN_2
    // TODO: 验证此方法是否正确
    int totalSize = cfg.plotChunkSize * 16;
    int roadWidth = 2; // 固定道路宽度为2

    if (plot1.x == plot2.x) {
        // 地皮在同一列,道路是水平的
        int minZ = std::min(plot1.z, plot2.z) * totalSize + totalSize - 1;
        int maxZ = minZ + roadWidth;
        int x    = plot1.x * totalSize + 1;
        return DiagonPos{
            Vec3{x,                 y, minZ    },
            Vec3{x + totalSize - 3, y, maxZ - 1}
        };
    } else {
        // 地皮在同一行,道路是垂直的
        int minX = std::min(plot1.x, plot2.x) * totalSize + totalSize - 1;
        int maxX = minX + roadWidth;
        int z    = plot1.z * totalSize + 1;
        return DiagonPos{
            Vec3{minX,     y, z                },
            Vec3{maxX - 1, y, z + totalSize - 3}
        };
    }
#endif
}

} // namespace plo