#ifdef REMOTE_API
#include "Remote.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/utils/JsonHelper.h"

namespace plo::remote {


void exportPLAPI() {
    using namespace RemoteCall;
    using FloatPos  = std::pair<Vec3, int>;
    string const sp = "PLAPI";

    exportAs(sp, "getPlotWorldDimid", []() -> int { return getPlotDimensionId(); });

    exportAs(sp, "getDimidFromString", [](string const& str) -> int { return VanillaDimensions::fromString(str); });

    exportAs(sp, "PlotPos_toString", [](int x, int z) -> string { return PlotPos{x, z}.toString(); });

    exportAs(sp, "PlotPos_getPlotID", [](int x, int z) -> string { return PlotPos{x, z}.getPlotID(); });

    exportAs(sp, "PlotPos_isPosInPlot", [](int px, int pz, FloatPos const& pos) -> bool {
        return PlotPos{px, pz}.isPosInPlot(pos.first);
    });

    exportAs(sp, "PlotPos_getSafestPos", [](int px, int pz) -> FloatPos {
        return std::make_pair(PlotPos{px, pz}.getSafestPos(), getPlotDimensionId());
    });

    exportAs(sp, "PlotPos_isPosOnBorder", [](int x, int z, FloatPos const& pos) -> bool {
        return PlotPos{x, z}.isPosOnBorder(pos.first);
    });

    using JS_PlotPos_Constructor = std::vector<int>;
    exportAs(sp, "getPlotPosByPos", [](FloatPos const& pos) -> JS_PlotPos_Constructor {
        JS_PlotPos_Constructor ret;
        PlotPos                pps{pos.first};
        ret.push_back(pps.mX);
        ret.push_back(pps.mZ);
        ret.push_back(pps.isValid());
        return ret;
    });

    exportAs(sp, "getPlotPosByXZ", [](int x, int z) -> JS_PlotPos_Constructor {
        JS_PlotPos_Constructor ret;
        PlotPos                pps{x, z};
        ret.push_back(pps.mX);
        ret.push_back(pps.mZ);
        ret.push_back(pps.isValid());
        return ret;
    });

    using namespace data;
    exportAs(sp, "getPlayerPermission", [](string const& uuid, PlotID const& pid, bool ignoreAdmin) -> int {
        auto&          db   = data::PlotBDStorage::getInstance();
        PlotPermission perm = db.getPlayerPermission(uuid, pid, ignoreAdmin);
        return static_cast<int>(perm);
    });

    exportAs(sp, "PlotMetadata_getPermissionTableConst", [](PlotID const& pid) -> string {
        auto& db = data::PlotBDStorage::getInstance();
        if (db.hasPlot(pid)) {
            auto meta = db.getPlot(pid);
            if (meta) {
                return utils::JsonHelper::structToJsonString(meta->getPermissionTableConst());
            }
        }
        return "";
    });
}


} // namespace plo::remote

#endif // REMOTE_API