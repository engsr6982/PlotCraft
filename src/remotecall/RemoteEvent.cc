#include "RemoteEvent.h"
#include "RemoteCallApi.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/utils/HashUtils.h"
#include "mc/world/actor/player/Player.h"
#include <cstddef>


using string = std::string;
using ll::hash_literals::operator""_h;
using namespace ll::hash_utils;

namespace plo::remotecall {

bool exportPLEventApi() {
    auto eventBus = &ll::event::EventBus::getInstance(); // 获取EventBus指针
    RemoteCall::exportAs(
        "PLAPI",        // 全局命名空间
        "PLEvent_Call", // 导出函数
        [eventBus](string const& event, string const& callbackName) -> bool {
            if (RemoteCall::hasFunc(event, callbackName)) {
                switch (doHash(event)) {
                case "PlayerEnterPlot"_h: {
                    // auto Call = RemoteCall::importAs<void(Player*)>(event, callbackName);
                    // TODO
                    return true;
                }
                case "PlayerLeavePlot"_h: {

                    return true;
                }
                default:
                    return false;
                }
            } else {
                return false; // not export
            }
        }
    );


    return true;
}


} // namespace plo::remotecall