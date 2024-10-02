#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"
#include "plotcraft/Global.h"
#include <optional>
#include <utility>
#include <vector>


namespace plo {


// 预定义
class PlotRoad;
class PlotCross;
class PlotPos;


enum class PlotDirection : int {
    Unknown = -1, // 未知
    North   = 0,  // 北     z-
    East    = 1,  // 东     x+
    South   = 2,  // 南     z+
    West    = 3,  // 西     x-
    NE      = 4,  // 东北   z- x+
    SE      = 5,  // 东南   z+ x+
    SW      = 6,  // 西南   z+ x-
    NW      = 7,  // 西北   z- x-
};


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
    PLAPI bool fixVertexs();                               // todo
    PLAPI std::vector<PlotPos> getRangedPlots() const;     // todo
    PLAPI std::vector<PlotRoad> getRangedRoads() const;    // todo
    PLAPI std::vector<PlotCross> getRangedCrosses() const; // todo
    PLAPI std::optional<PlotPos> tryMerge(PlotPos const& other); // 尝试合并两个多边形，如果没有值则合并失败 // todo
};


class PlotRoad {
public:
    int           mX, mZ;        // 道路坐标 (注意：此坐标会重复, 因为地皮x,z正方向有两个道路)
    DiagonPos     mDiagonPos;    // 对角线坐标
    bool          mIsMergedPlot; // 是否是合并的地皮
    bool          mValid;        // 是否有效
    PlotDirection mDirection;    // 道路方向 (East: x+ 道路横向, South: z+ 道路纵向 (方向以x+为基准))

    PlotRoad() = delete;
    PLAPI PlotRoad(Vec3 const& vec3);

    /**
     * @brief 创建道路
     * @param x 道路x坐标
     * @param z 道路z坐标
     * @param direction 道路方向，请传递 East 或 South 其余抛出异常
     */
    [[deprecated]] PLAPI PlotRoad(int x, int z, PlotDirection direction);

    PLAPI bool isValid() const; // 判断是否有效

    PLAPI string toString() const;
    PLAPI RoadID getRoadID() const;

    PLAPI bool hasPoint(BlockPos const& pos) const;                 // 判断一个点是否在道路内
    PLAPI void fill(Block const& block, bool removeBorder = false); // 填充道路

    PLAPI std::vector<PlotCross> getAdjacentCrosses() const;
    PLAPI bool                   isAdjacent(PlotCross const& cross) const;
    PLAPI std::vector<PlotDirection> getAfterFillingNeedFixBorderDirections() const;
};

class PlotCross {
public:
    int       mX, mZ;
    DiagonPos mDiagonPos;
    bool      mIsMergedPlot; // 是否是合并的地皮
    bool      mValid;        // 是否有效

    PlotCross() = delete;
    PLAPI PlotCross(int x, int z);
    PLAPI PlotCross(const Vec3& vec3);

    PLAPI bool isValid() const; // 判断是否有效

    PLAPI string  toString() const;
    PLAPI CrossID getCrossID() const;

    PLAPI bool hasPoint(BlockPos const& pos) const;                 // 判断一个点是否在路口内
    PLAPI void fill(Block const& block, bool removeBorder = false); // 填充路口

    PLAPI std::vector<PlotRoad> getAdjacentRoads() const;
    PLAPI bool                  isAdjacent(PlotRoad const& road) const;
};


} // namespace plo