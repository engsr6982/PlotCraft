#pragma once
#include "PlotDirection.h"
#include "mc/world/level/block/Block.h"
#include "plotcraft/Global.h"
#include <vector>


namespace plot {

class PlotCross;

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
};


} // namespace plot