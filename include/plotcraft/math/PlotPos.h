#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"
#include "plotcraft/Global.h"
#include <memory>
#include <vector>


namespace plot {


class PlotRoad;
class PlotCross;
class PlotPos;

class PlotPos {
public:
    int     mX, mZ;   // 地皮坐标
    Vertexs mVertexs; // 地皮顶点

    PlotPos() = default;
    PLAPI PlotPos(int x, int z);
    PLAPI PlotPos(const Vec3& vec3);

    PLAPI bool isValid() const; // 判断是否有效

    PLAPI string toString() const; // 转换为字符串

    PLAPI PlotID getPlotID() const; // 获取地皮ID

    PLAPI int getSurfaceY() const; // 获取地表Y坐标

    PLAPI Vec3 getSafestPos() const; // 获取最安全的位置

    PLAPI bool isPosInPlot(const Vec3& vec3) const; // 判断一个点是否在地皮内

    PLAPI bool isPosOnBorder(Vec3 const& vec3) const; // 判断一个点是否在地皮边界上
    PLAPI bool isAABBOnBorder(BlockPos const& min, BlockPos const& max) const; // 判断一个立方体是否在地皮边界上
    PLAPI bool isRadiusOnBorder(BlockPos const& center, int radius) const; // 判断一个圆是否在地皮边界上

    PLAPI bool operator==(PlotPos const& other) const;
    PLAPI bool operator!=(PlotPos const& other) const;

    // static
    PLAPI static int  getSurfaceYStatic();
    PLAPI static bool isAdjacent(PlotPos const& plot1, PlotPos const& plot2);      // 判断两个地皮是否相邻
    PLAPI static bool isPointInPolygon(const Vec3& point, Vertexs const& polygon); // 判断一个点是否在多边形内

    PLAPI static Vertexs              getAABBVertexs(BlockPos const& min, BlockPos const& max);
    PLAPI static std::vector<PlotPos> getPlotPosAt(BlockPos const& min, BlockPos const& max);
    PLAPI static std::vector<PlotPos> getPlotPosAt(BlockPos const& center, int radius);
    PLAPI static bool
    isAABBCollision(BlockPos const& min1, BlockPos const& max1, BlockPos const& min2, BlockPos const& max2);

    PLAPI static void fillAABB(BlockPos const& min, BlockPos const& max, Block const& block);

    // MergeAPI:
    PLAPI bool isAdjacent(PlotRoad const& road) const;
    PLAPI bool isCorner(PlotCross const& cross) const;
    PLAPI void fixBorder();
    PLAPI std::vector<PlotPos> getRangedPlots() const;
    PLAPI std::vector<PlotRoad> getRangedRoads() const;
    PLAPI std::vector<PlotCross> getRangedCrosses() const;
    PLAPI std::unique_ptr<PlotPos> tryMerge(PlotPos const& other) const; // 尝试合并两个多边形，如果没有值则合并失败
};


} // namespace plot