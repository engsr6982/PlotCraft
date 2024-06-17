#pragma once
#include "mc/math/Vec3.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"

namespace plo {

class PlotPos {
public:
    int  x, z;           // 地皮坐标
    Vec3 minPos;         // 地皮小端坐标
    Vec3 maxPos;         // 地皮大端坐标
    bool mIsValid{true}; // 地皮是否有效

    PLAPI PlotPos();

    PLAPI PlotPos(int x, int z);

    PLAPI PlotPos(const Vec3& vec3);

    // 地皮是否有效(无效则代表该坐标没有对应的地皮)
    PLAPI bool isValid();

    PLAPI Vec3 getMin();

    PLAPI Vec3 getMax();

    PLAPI string toString();

    PLAPI string toDebug();

    PLAPI bool isPosInPlot(const Vec3& vec3);

    PLAPI std::vector<PlotPos> getAdjacentPlots();

    // 重载比较运算符
    PLAPI bool operator==(const PlotPos& other) const;
    PLAPI bool operator!=(const PlotPos& other) const;
};


} // namespace plo