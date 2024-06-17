#pragma once
#include "../config/Config.h"
#include "Macro.h"
#include "mc/math/Vec3.h"

namespace plo::core {


class PlotPos {
public:
    int  x, z;           // 地皮坐标
    Vec3 minPos;         // 地皮小端坐标
    Vec3 maxPos;         // 地皮大端坐标
    bool mIsValid{true}; // 地皮是否有效

    PLOAPI PlotPos();

    PLOAPI PlotPos(int x, int z);

    PLOAPI PlotPos(const Vec3& vec3);

    // 地皮是否有效(无效则代表该坐标没有对应的地皮)
    PLOAPI bool isValid();

    PLOAPI Vec3 getMin();

    PLOAPI Vec3 getMax();

    PLOAPI string toString();

    PLOAPI string toDebug();

    PLOAPI bool isPosInPlot(const Vec3& vec3);

    PLOAPI std::vector<PlotPos> getAdjacentPlots();

    // 重载比较运算符
    PLOAPI bool operator==(const PlotPos& other) const;
    PLOAPI bool operator!=(const PlotPos& other) const;
};


} // namespace plo::core