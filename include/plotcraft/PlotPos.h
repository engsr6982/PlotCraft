#pragma once
#include "mc/math/Vec3.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"

namespace plo {

// Disable C4244
#pragma warning(disable : 4244)

class PlotPos {
public:
    int  x, z;           // 地皮坐标
    Vec3 minPos;         // 地皮小端坐标
    Vec3 maxPos;         // 地皮大端坐标
    bool mIsValid{true}; // 地皮是否有效

    PLAPI PlotPos();
    PLAPI PlotPos(int x, int z);
    PLAPI PlotPos(const Vec3& vec3);

    PLAPI bool isValid() const;

    PLAPI Vec3 getMin() const;

    PLAPI Vec3 getMax() const;

    PLAPI string toString() const;

    PLAPI string getPlotID() const;

    PLAPI string toDebug() const;

    PLAPI bool isPosInPlot(const Vec3& vec3) const;

    PLAPI Vec3 getSafestPos() const;

    PLAPI void tryFixMinAndMaxPos();

    PLAPI bool isPosOnBorder(const Vec3& vec3);

    PLAPI std::vector<PlotPos> getAdjacentPlots() const;

    PLAPI bool operator==(PlotPos const& other) const;
    PLAPI bool operator!=(PlotPos const& other) const;
};


} // namespace plo