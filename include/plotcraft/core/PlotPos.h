#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/block/Block.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"
#include <utility>


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
//     PLAPI std::vector<class PlotPos> getAdjacentPlots() const;
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


class PlotPos {
public:
    int               mX, mZ;   // 地皮坐标
    std::vector<Vec3> mVertexs; // 地皮顶点

    PLAPI PlotPos();
    PLAPI PlotPos(int x, int z);
    PLAPI PlotPos(const Vec3& vec3);
    PLAPI PlotPos(int x, int z, const std::vector<Vec3>& vertexs);

    PLAPI bool isValid() const;

    PLAPI string toString() const;

    PLAPI string getPlotID() const;

    PLAPI int getSurfaceY() const;

    PLAPI Vec3 getSafestPos() const;

    PLAPI bool isPosInPlot(const Vec3& vec3) const;

    PLAPI bool isPosOnBorder(const Vec3& vec3) const;

    // PLAPI bool canMerge(PlotPos& other) const;        // TODO
    // PLAPI bool checkAndFixVertexs();                  // TODO
    // PLAPI bool tryMergeAndFixVertexs(PlotPos& other); // TODO

    PLAPI std::vector<PlotPos> getAdjacentPlots() const; // 获取相邻的地皮
    // PLAPI std::vector<PlotRoadPos> getAdjacentRoads() const; // TODO: 获取相邻的道路

    // PLAPI std::vector<PlotRoadPos> getRangedRoads() const;    // TODO: 获取范围内的道路
    // PLAPI std::vector<PlotCrossPos> getRangedCrosses() const; // TODO: 获取范围内的路口

    PLAPI bool operator==(PlotPos const& other) const;
    PLAPI bool operator!=(PlotPos const& other) const;


    // static
    PLAPI static bool isAdjacent(PlotPos const& plot1, PlotPos const& plot2);

    PLAPI static bool isPointInPolygon(const Vec3& point, const std::vector<Vec3>& polygon);
};


} // namespace plo