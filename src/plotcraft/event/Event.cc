#include "Event.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/enums/TextPacketType.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include "plotcraft/core/PlotPos.h"
#include "plugin/MyPlugin.h"
#include <string>
#include <thread>


using string = std::string;
using ll::chrono_literals::operator""_tick;

ll::schedule::GameTickScheduler mTickScheduler; // Tick调度


namespace plo::event {


void registerEventListener() {
    mTickScheduler.add<ll::schedule::RepeatTask>(5_tick, []() {
        std::thread([] {
            Level& lv = *ll::service::getLevel();
            lv.forEachPlayer([](Player& p) {
                if (p.getDimension().mName != "plot") return true; // 不是同一维度
                if (p.isLoading() || p.isSimulated() || p.isSimulatedPlayer()) return true;
                auto pps = core::PlotPos(p.getPosition()).toDebug();

                // §a✔§r
                // §c✘§r

                // Tip1:
                // 地皮: (0,1) | (0,0,0) => (16,16,16)
                // 主人: xxx  |  名称: xxx
                // 出售: x/√  |  价格: xxx

                // Tip2:
                // 地皮: (0,1) | (0,0,0) => (16,16,16)
                // 主人: 无主  | 价格: xxx
                // 输入：/plo buy 购买

                TextPacket pkt = TextPacket();
                pkt.mType      = TextPacketType::Tip;


                pkt.mMessage   = pps;


                p.sendNetworkPacket(pkt);
                return true;
            });
        }).join();
    });
}


void unRegisterEventListener() { mTickScheduler.clear(); }


} // namespace plo::event