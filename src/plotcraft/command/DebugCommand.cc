#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPositionFloat.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/world/actor/Actor.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/math/PlotPos.h"
#include <sstream>

namespace plot::command {

struct DParam {
    CommandBlockName name;
    bool             remove_border;
};


struct DTestClass2 {
    CommandPositionFloat road_pos;
    CommandPositionFloat cross_pos;
};

void SetupDebugCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(COMMAND_NAME);

    cmd.overload<DParam>()
        .text("debug_fill_cross")
        .required("name")
        .required("remove_border")
        .execute([](CommandOrigin const& ori, CommandOutput& out, DParam const& param) {
            auto ent = ori.getEntity();
            if (!ent || !ent->isPlayer()) {
                out.error("Must be a player");
                return;
            }

            auto bl = param.name.resolveBlock(param.name.id).getBlock();
            if (!bl) {
                out.error("Could not find block");
                return;
            }

            PlotCross cross = PlotCross(ent->getPosition());
            if (cross.isValid()) {
                cross.fill(*bl, param.remove_border);
            } else {
                out.error("Invalid cross");
            }
        });

    cmd.overload<DParam>()
        .text("debug_fill_road")
        .required("name")
        .required("remove_border")
        .execute([](CommandOrigin const& ori, CommandOutput& out, DParam const& param) {
            auto ent = ori.getEntity();
            if (!ent || !ent->isPlayer()) {
                out.error("Must be a player");
                return;
            }

            auto bl = param.name.resolveBlock(param.name.id).getBlock();
            if (!bl) {
                out.error("Could not find block");
                return;
            }

            PlotRoad road = PlotRoad(ent->getPosition());
            if (road.isValid()) {
                road.fill(*bl, param.remove_border);
            } else {
                out.error("Invalid road");
            }
        });

    cmd.overload().text("debug_fix_border").execute([](CommandOrigin const& ori, CommandOutput& out) {
        auto ent = ori.getEntity();
        if (!ent || !ent->isPlayer()) {
            out.error("Must be a player");
            return;
        }

        PlotPos pos = PlotPos(ent->getPosition());
        if (pos.isValid()) {
            pos.fixBorder();
        } else {
            out.error("Invalid PlotPos");
        }
    });

    cmd.overload().text("debug_get_adjacent_road").execute([](CommandOrigin const& ori, CommandOutput& out) {
        Actor* ent = ori.getEntity();
        if (!ent || !ent->isPlayer()) {
            out.error("Must be a player");
            return;
        }

        PlotCross cross = PlotCross(ent->getPosition());
        if (cross.isValid()) {
            auto   roads   = cross.getAdjacentRoads();
            string out_str = "Adjacent roads: \n";
            for (auto const& road : roads) {
                out_str += road.toString() + "\n";
            }
            out.success(out_str);
        } else {
            out.error("Invalid cross");
        }
    });

    cmd.overload().text("debug_get_adjacent_cross").execute([](CommandOrigin const& ori, CommandOutput& out) {
        Actor* ent = ori.getEntity();
        if (!ent || !ent->isPlayer()) {
            out.error("Must be a player");
            return;
        }

        PlotRoad road = PlotRoad(ent->getPosition());
        if (road.isValid()) {
            auto   crosses = road.getAdjacentCrosses();
            string out_str = "Adjacent crosses: \n";
            for (auto const& cross : crosses) {
                out_str += cross.toString() + "\n";
            }
            out.success(out_str);
        } else {
            out.error("Invalid road");
        }
    });

    cmd.overload<DTestClass2>()
        .text("debug_is_adjacent")
        .required("road_pos")
        .required("cross_pos")
        .execute([](CommandOrigin const& ori, CommandOutput& out, DTestClass2 const& param) {
            Actor* ent = ori.getEntity();
            if (!ent || !ent->isPlayer()) {
                out.error("Must be a player");
                return;
            }

            auto road_pos  = param.road_pos.getBlockPos(CommandVersion::CurrentVersion, ori);
            auto cross_pos = param.cross_pos.getBlockPos(CommandVersion::CurrentVersion, ori);

            PlotRoad  road(road_pos);
            PlotCross cross(cross_pos);

            if (!road.isValid() || !cross.isValid()) {
                out.error("Invalid road or cross");
                return;
            }

            std::ostringstream out_str;
            out_str << "Road => Cross: " << road.isAdjacent(cross) << "\n";
            out_str << "Cross => Road: " << cross.isAdjacent(road) << "\n";
            out.success(out_str.str());
        });

    cmd.overload().text("debug_reset_db").execute([](CommandOrigin const& ori, CommandOutput& out) {
        auto& db   = data::PlotDBStorage::getInstance();
        auto& impl = db.getDB();

        impl.iter([&impl](auto k, auto v) {
            impl.del(k);
            return true;
        });

        db.load();
    });
}


} // namespace plot::command