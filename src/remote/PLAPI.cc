#include "Remote.h"


namespace plo::remote {

int getPlotWorldDimid() { return VanillaDimensions::fromString("plot"); }

void exportPLAPI() {
    using namespace RemoteCall;
    using FloatPos  = std::pair<Vec3, int>;
    string const sp = "PLAPI";

    exportAs(sp, "getPlotWorldDimid", []() -> int { return getPlotWorldDimid(); });

    exportAs(sp, "PlotPos_toString", [](int x, int z) -> string { return PlotPos{x, z}.toString(); });

    exportAs(sp, "PlotPos_toDebug", [](int x, int z) -> string { return PlotPos{x, z}.toDebug(); });

    exportAs(sp, "PlotPos_isPosInPlot", [](int px, int pz, FloatPos const& pos) -> bool {
        return PlotPos{px, pz}.isPosInPlot(pos.first);
    });

    exportAs(sp, "PlotPos_getSafestPos", [](int px, int pz) -> FloatPos {
        return std::make_pair(PlotPos{px, pz}.getSafestPos(), getPlotWorldDimid());
    });

    exportAs(sp, "PlotPos_isPosOnBorder", [](int x, int z, FloatPos const& pos) -> bool {
        return PlotPos{x, z}.isPosOnBorder(pos.first);
    });

    exportAs(sp, "PlotPos_getMin", [](int x, int z) -> FloatPos {
        return std::make_pair(PlotPos{x, z}.getMin(), getPlotWorldDimid());
    });

    exportAs(sp, "PlotPos_getMax", [](int x, int z) -> FloatPos {
        return std::make_pair(PlotPos{x, z}.getMax(), getPlotWorldDimid());
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

    using namespace database;
    exportAs(
        sp,
        "getPlayerPermission",
        [](string const& uuid, PlotID const& pid, bool ignoreAdmin, bool ignoreCache) -> int {
            auto const     uid  = UUID::fromString(uuid);
            auto&          db   = database::PlotDB::getInstance();
            PlotPermission perm = db.getPermission(uid, pid, ignoreAdmin, ignoreCache);
            return static_cast<int>(perm);
        }
    );
}


} // namespace plo::remote