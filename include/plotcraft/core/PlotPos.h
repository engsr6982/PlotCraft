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
    int               mX, mZ;   // 地皮坐标
    std::vector<Vec3> mVertexs; // 地皮顶点

    PLAPI PlotPos();
    PLAPI PlotPos(int x, int z);
    PLAPI PlotPos(const Vec3& vec3);

    PLAPI bool isValid() const;

    PLAPI string toString() const;

    PLAPI string getPlotID() const;

    PLAPI int getSurfaceY() const;

    PLAPI Vec3 getSafestPos() const;

    PLAPI bool isPosInPlot(const Vec3& vec3) const;

    PLAPI bool isPosOnBorder(const Vec3& vec3) const;

    PLAPI std::vector<PlotPos> getAdjacentPlots() const;

    PLAPI bool operator==(PlotPos const& other) const;
    PLAPI bool operator!=(PlotPos const& other) const;


    // static
    PLAPI static bool isAdjacent(PlotPos const& plot1, PlotPos const& plot2);

    PLAPI static bool isPointInPolygon(const Vec3& point, const std::vector<Vec3>& polygon);
};


} // namespace plo