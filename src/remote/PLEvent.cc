#ifdef REMOTE_API
#include "Remote.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/utils/HashUtils.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/event/PlotEvents.h"


using ll::hash_literals::operator""_h;
using namespace ll::hash_utils;
using namespace RemoteCall;
using FloatPos               = std::pair<Vec3, int>;
using JS_PlotPos_Constructor = std::vector<int>; // x,z,isValid


namespace plo::remote {

void exportPLEvent() {
    string const sp  = "PLAPI";
    auto*        bus = &ll::event::EventBus::getInstance();


    exportAs(sp, "CallPLEvent", [bus](string const& eventName, string const& funcName) -> bool {
        if (hasFunc(eventName, funcName)) {
            switch (doHash(eventName)) {
            case "PlayerEnterPlot"_h: {
                auto call = importAs<void(Player * player, JS_PlotPos_Constructor constructor)>(eventName, funcName);
                bus->emplaceListener<event::PlayerEnterPlot>([call](event::PlayerEnterPlot& ev) {
                    auto                   pos  = ev.getPos();
                    JS_PlotPos_Constructor cons = {pos.x, pos.z, pos.isValid()};

                    try {
                        call(ev.getPlayer(), cons);
                    } catch (...) {}
                });
                return true;
            }
            case "PlayerLeavePlot"_h: {
                auto call = importAs<void(Player * player, JS_PlotPos_Constructor constructor)>(eventName, funcName);
                bus->emplaceListener<event::PlayerLeavePlot>([call](event::PlayerLeavePlot& ev) {
                    auto                   pos  = ev.getPos();
                    JS_PlotPos_Constructor cons = {pos.x, pos.z, pos.isValid()};

                    try {
                        call(ev.getPlayer(), cons);
                    } catch (...) {}
                });
                return true;
            }
            }
        }
        return false;
    });
}


} // namespace plo::remote

#endif // REMOTE_API