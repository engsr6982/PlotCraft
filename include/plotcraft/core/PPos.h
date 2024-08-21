#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"


namespace plo {

// TODO
// using DiagonPos = std::pair<Vec3, Vec3>;
// enum class PlotDirection : int {
//     North = 0, // 北
//     East  = 1, // 东
//     South = 2, // 南
//     West  = 3  // 西
// };
// class PlotRoadPos {
// public:
//     int       mX, mZ;
//     DiagonPos mDiagonPos;
//     bool      mIsMergedPlot; // 是否是合并的地皮
//     PLAPI PlotRoadPos();
//     PLAPI PlotRoadPos(int x, int z);
//     PLAPI PlotRoadPos(const Vec3& vec3);
//     PLAPI PlotRoadPos(int x, int z, const DiagonPos& diagonPos, bool isMergedPlot);
//     PLAPI string toString() const;
//     PLAPI string getRoadID() const;
//     PLAPI bool fillRoad(Block& block, bool includeBorder = false);
//     PLAPI std::vector<class PPos> getAdjacentPlots() const;
//     PLAPI static bool isAdjacent(const PlotRoadPos& road1, const PlotRoadPos& road2);
// };
// class PlotCrossPos {
// public:
//     int       mX, mZ;
//     DiagonPos mDiagonPos;
//     bool      mIsMergedPlot; // 是否是合并的地皮
//     PLAPI PlotCrossPos();
//     PLAPI PlotCrossPos(int x, int z);
//     PLAPI PlotCrossPos(const Vec3& vec3);
//     PLAPI PlotCrossPos(int x, int z, const DiagonPos& diagonPos, bool isMergedPlot);
//     PLAPI string toString() const;
//     PLAPI string getCrossID() const;
//     PLAPI bool fillCross(Block& block);
//     PLAPI std::vector<PlotRoadPos> getAdjacentRoads() const;
// };

using Vertexs = std::vector<Vec3>; // 顶点


class PPos {
public:
    int     mX, mZ;   // 地皮坐标
    Vertexs mVertexs; // 地皮顶点

    PLAPI PPos();
    PLAPI PPos(int x, int z);
    PLAPI PPos(const Vec3& vec3);
    PLAPI PPos(int x, int z, Vertexs const& vertexs);

    PLAPI bool isValid() const;

    PLAPI string toString() const;

    PLAPI string getPlotID() const;

    PLAPI int getSurfaceY() const;

    PLAPI Vec3 getSafestPos() const;

    PLAPI bool isPosInPlot(const Vec3& vec3) const;

    PLAPI bool isPosOnBorder(const Vec3& vec3) const;
    PLAPI bool isCubeOnBorder(class Cube const& cube) const;

    // PLAPI bool canMerge(PPos& other) const;        // TODO
    // PLAPI bool checkAndFixVertexs();                  // TODO
    // PLAPI bool tryMergeAndFixVertexs(PPos& other); // TODO

    // PLAPI std::vector<PPos> getRangedPlots() const;           // TODO: 获取范围内的地皮
    // PLAPI std::vector<PlotRoadPos> getRangedRoads() const;    // TODO: 获取范围内的道路
    // PLAPI std::vector<PlotCrossPos> getRangedCrosses() const; // TODO: 获取范围内的路口

    PLAPI bool operator==(PPos const& other) const;
    PLAPI bool operator!=(PPos const& other) const;

    // static
    PLAPI static bool isAdjacent(PPos const& plot1, PPos const& plot2);

    PLAPI static bool isPointInPolygon(const Vec3& point, Vertexs const& polygon);
};


class Cube {
public:
    BlockPos mMin, mMax;

    Cube() = delete;
    Cube(BlockPos const& min, BlockPos const& max);

    Vertexs get2DVertexs() const; // 获取2D顶点

    bool hasPos(BlockPos const& pos) const; // 判断一个点是否在立方体内

    std::vector<PPos> getRangedPlots() const; // 获取范围内的地皮

    bool operator==(const Cube& other) const;
    bool operator!=(const Cube& other) const;

    // static
    static bool isCollision(Cube const& cube1, Cube const& cube2); // 判断两个立方体是否碰撞
};

} // namespace plo