#pragma once
#include "../config/Config.h"
#include "mc/math/Vec3.h"

namespace plo::core {


class PlotPos {
public:
    int  x, z;           // 地皮坐标
    Vec3 minPos;         // 地皮小端坐标
    Vec3 maxPos;         // 地皮大端坐标
    bool mIsValid{true}; // 地皮是否有效

    PlotPos();

    PlotPos(int x, int z);

    PlotPos(const Vec3& vec3);

    // 地皮是否有效(无效则代表该坐标没有对应的地皮)
    bool isValid();

    Vec3 getMin();

    Vec3 getMax();

    string toString();

    string toDebug();

    bool isPosInPlot(const Vec3& vec3);

    std::vector<PlotPos> getAdjacentPlots();

    // 重载比较运算符
    bool operator==(const PlotPos& other) const;
    bool operator!=(const PlotPos& other) const;
};


} // namespace plo::core