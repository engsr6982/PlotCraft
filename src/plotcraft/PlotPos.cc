#include "plotcraft/PlotPos.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "plotcraft/Config.h"
#include <cmath>

#include "plotcraft/data/PlotBDStorage.h"

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
    // 兼容地皮合并
    auto const& db = data::PlotBDStorage::getInstance();
    auto        id = getPlotID();

    if (db.isMergedPlot(id)) id = db.getOwnerPlotID(id);
    if (db.hasMergedPlotInfo(id)) {
        auto const& info = db.getMergedPlotInfoConst(id);
        minPos           = Vec3{info.mMinX, -64, info.mMinZ};
        maxPos           = Vec3{info.mMaxX, 320, info.mMaxZ};
        return;
    }

    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotWidth + cfg.roadWidth;
    minPos          = Vec3{x * totalSize, -64, z * totalSize};
    maxPos          = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
}

PlotPos::PlotPos(const Vec3& vec3) {
    auto& cfg       = config::cfg.generator;
    int   totalSize = cfg.plotWidth + cfg.roadWidth;

    x = std::floor(vec3.x / totalSize);
    z = std::floor(vec3.z / totalSize);

    // 兼容地皮合并
    auto const& db = data::PlotBDStorage::getInstance();
    auto        id = getPlotID();
    if (db.isMergedPlot(id)) id = db.getOwnerPlotID(id);
    if (db.hasMergedPlotInfo(id)) {
        auto const& info = db.getMergedPlotInfoConst(id);
        minPos           = Vec3{info.mMinX, -64, info.mMinZ};
        maxPos           = Vec3{info.mMaxX, 320, info.mMaxZ};
        return;
    }

    int localX = static_cast<int>(std::floor(vec3.x)) % totalSize;
    int localZ = static_cast<int>(std::floor(vec3.z)) % totalSize;

    if (localX < 0) localX += totalSize;
    if (localZ < 0) localZ += totalSize;

    if (localX >= cfg.plotWidth || localZ >= cfg.plotWidth) {
        minPos   = Vec3{0, 0, 0};
        maxPos   = Vec3{0, 0, 0};
        x        = 0;
        z        = 0;
        mIsValid = false; // 无效的地皮点
    } else {
        minPos = Vec3{x * totalSize, -64, z * totalSize};
        maxPos = Vec3{minPos.x + cfg.plotWidth - 1, 320, minPos.z + cfg.plotWidth - 1};
    }
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
}

} // namespace plo