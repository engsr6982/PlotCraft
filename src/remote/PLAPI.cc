#ifdef REMOTE_API
#include "Remote.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/data/PlotBDStorage.h"

namespace plo::remote {


void exportPLAPI() {
    using namespace RemoteCall;
    using FloatPos  = std::pair<Vec3, int>;
    string const sp = "PLAPI";

    exportAs(sp, "getPlotWorldDimid", []() -> int { return getPlotDimensionId(); });

    exportAs(sp, "getDimidFromString", [](string const& str) -> int { return VanillaDimensions::fromString(str); });

    exportAs(sp, "PlotPos_toString", [](int x, int z) -> string { return PlotPos{x, z}.toString(); });

    exportAs(sp, "PlotPos_getPlotID", [](int x, int z) -> string { return PlotPos{x, z}.getPlotID(); });

    exportAs(sp, "PlotPos_toDebug", [](int x, int z) -> string { return PlotPos{x, z}.toDebug(); });

    exportAs(sp, "PlotPos_isPosInPlot", [](int px, int pz, FloatPos const& pos) -> bool {
        return PlotPos{px, pz}.isPosInPlot(pos.first);
    });

    exportAs(sp, "PlotPos_getSafestPos", [](int px, int pz) -> FloatPos {
        return std::make_pair(PlotPos{px, pz}.getSafestPos(), getPlotDimensionId());
    });

    exportAs(sp, "PlotPos_isPosOnBorder", [](int x, int z, FloatPos const& pos) -> bool {
        return PlotPos{x, z}.isPosOnBorder(pos.first);
    });

    exportAs(sp, "PlotPos_getMin", [](int x, int z) -> FloatPos {
        return std::make_pair(PlotPos{x, z}.getMin(), getPlotDimensionId());
    });

    exportAs(sp, "PlotPos_getMax", [](int x, int z) -> FloatPos {
        return std::make_pair(PlotPos{x, z}.getMax(), getPlotDimensionId());
    });

    using JS_PlotPos_Constructor = std::vector<int>;
    exportAs(sp, "getPlotPosByPos", [](FloatPos const& pos) -> JS_PlotPos_Constructor {
        JS_PlotPos_Constructor ret;
        PlotPos                pps{pos.first};
        ret.push_back(pps.x);
        ret.push_back(pps.z);
        ret.push_back(pps.isValid());
        return ret;
    });

    exportAs(sp, "getPlotPosByXZ", [](int x, int z) -> JS_PlotPos_Constructor {
        JS_PlotPos_Constructor ret;
        PlotPos                pps{x, z};
        ret.push_back(pps.x);
        ret.push_back(pps.z);
        ret.push_back(pps.isValid());
        return ret;
    });

    using namespace data;
    exportAs(sp, "getPlayerPermission", [](string const& uuid, PlotID const& pid, bool ignoreAdmin) -> int {
        auto&          db   = data::PlotBDStorage::getInstance();
        PlotPermission perm = db.getPlayerPermission(uuid, pid, ignoreAdmin);
        return static_cast<int>(perm);
    });
}


} // namespace plo::remote

#endif // REMOTE_API