#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"
#include "plotcraft/Global.h"
#include <vector>


namespace plot {

class PlotRoad;

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


} // namespace plot