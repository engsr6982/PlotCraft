#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "plotcraft/math/PlotPos.h"

namespace plo::command {

struct DParam {
    CommandBlockName name;
    bool             remove_border;
};

void SetupDebugCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo");

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
}


} // namespace plo::command