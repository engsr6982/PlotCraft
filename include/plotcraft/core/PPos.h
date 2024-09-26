#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"


namespace plo {

using DiagonPos = std::pair<Vec3, Vec3>;
using Vertexs   = std::vector<Vec3>;


class PlotRoadPos {
public:
    int       mX, mZ;        // 道路坐标
    DiagonPos mDiagonPos;    // 对角线坐标
    bool      mIsMergedPlot; // 是否是合并的地皮

    PLAPI PlotRoadPos();
    PLAPI PlotRoadPos(int x, int z);
    PLAPI PlotRoadPos(const Vec3& vec3);
    PLAPI PlotRoadPos(int x, int z, const DiagonPos& diagonPos, bool isMergedPlot);

    PLAPI string toString() const;
    PLAPI string getRoadID() const;

    PLAPI bool isOnRoad(BlockPos const& pos) const;
    PLAPI bool fillRoad(Block const& block, bool includeBorder = false);

    PLAPI std::vector<class PPos> getAdjacentPlots() const;
    PLAPI static bool             isAdjacent(const PlotRoadPos& road1, const PlotRoadPos& road2);
};

class PlotCrossPos {
public:
    int       mX, mZ;
    DiagonPos mDiagonPos;
    bool      mIsMergedPlot; // 是否是合并的地皮

    PLAPI PlotCrossPos();
    PLAPI PlotCrossPos(int x, int z);
    PLAPI PlotCrossPos(const Vec3& vec3);
    PLAPI PlotCrossPos(int x, int z, const DiagonPos& diagonPos, bool isMergedPlot);

    PLAPI string toString() const;
    PLAPI string getCrossID() const;

    PLAPI bool isOnCross(BlockPos const& pos) const;
    PLAPI bool fillCross(Block const& block);
    PLAPI std::vector<PlotRoadPos> getAdjacentRoads() const;
};


class PPos {
public:
    int     mX, mZ;   // 地皮坐标
    Vertexs mVertexs; // 地皮顶点

    PLAPI PPos();
    PLAPI PPos(int x, int z);
    PLAPI PPos(const Vec3& vec3);
    PLAPI PPos(int x, int z, Vertexs const& vertexs);

    PLAPI bool isValid() const; // 判断是否有效

    PLAPI string toString() const; // 转换为字符串

    PLAPI string getPlotID() const; // 获取地皮ID

    PLAPI int getSurfaceY() const; // 获取地表Y坐标

    PLAPI Vec3 getSafestPos() const; // 获取最安全的位置

    PLAPI bool isPosInPlot(const Vec3& vec3) const; // 判断一个点是否在地皮内

    PLAPI bool isPosOnBorder(const Vec3& vec3) const;              // 判断一个点是否在地皮边界上
    PLAPI bool isCubeOnBorder(class Cube const& cube) const;       // 判断一个立方体是否在地皮边界上
    PLAPI bool isRadiusOnBorder(class Radius const& radius) const; // 判断一个圆是否在地皮边界上

    PLAPI bool fixVertexs(); // 修正顶点

    PLAPI bool canMerge(PPos& other) const; // 判断两个地皮是否可以合并

    PLAPI std::vector<PPos> getRangedPlots() const;           // 获取范围内的地皮
    PLAPI std::vector<PlotRoadPos> getRangedRoads() const;    // 获取范围内的道路
    PLAPI std::vector<PlotCrossPos> getRangedCrosses() const; // 获取范围内的路口

    PLAPI bool operator==(PPos const& other) const;
    PLAPI bool operator!=(PPos const& other) const;

    // static
    PLAPI static bool isAdjacent(PPos const& plot1, PPos const& plot2); // 判断两个地皮是否相邻

    PLAPI static bool isPointInPolygon(const Vec3& point, Vertexs const& polygon); // 判断一个点是否在多边形内
};


class Cube {
public:
    BlockPos mMin, mMax; // 最小最大坐标

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


class Radius {
public:
    BlockPos mCenter; // 中心点
    int      mRadius; // 半径

    Radius() = delete;
    Radius(BlockPos const& center, int radius) : mCenter(center), mRadius(radius){};

    std::vector<PPos> getRangedPlots() const; // 获取范围内的地皮

    bool operator==(const Radius& other) const;
    bool operator!=(const Radius& other) const;
};


} // namespace plo