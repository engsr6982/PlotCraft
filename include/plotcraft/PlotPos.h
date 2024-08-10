#pragma once
#include "mc/math/Vec3.h"
#include "plotcraft/Config.h"
#include "plotcraft/Macro.h"
#include <utility>

namespace plo {

// Disable C4244
#pragma warning(disable : 4244)

enum class Direction : int {
    North = 0, // 北
    East  = 1, // 东
    South = 2, // 南
    West  = 3  // 西
};

using DiagonPos = std::pair<Vec3, Vec3>;

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

    [[deprecated]] PLAPI Vec3 getMin() const;
    [[deprecated]] PLAPI Vec3 getMax() const;

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

    PLAPI int getSurfaceY() const; // 获取地皮地表Y坐标

    PLAPI DiagonPos getDiagonPos() const;

    PLAPI DiagonPos getBorderDiagonPos(Direction direction) const; // 获取地皮边框对角坐标


    // static
    // 是否是相邻地皮
    PLAPI static bool isAdjacent(PlotPos const& plot1, PlotPos const& plot2);

    // 获取相邻地皮的道路对角坐标
    PLAPI static DiagonPos getAdjacentPlotRoad(PlotPos const& plot1, PlotPos const& plot2);
};


} // namespace plo