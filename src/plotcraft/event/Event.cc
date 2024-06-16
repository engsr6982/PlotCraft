#include "Event.h"
#include "fmt/format.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/enums/TextPacketType.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include "plot/PlayerPlotEvent.h"
#include "plotcraft/config/Config.h"
#include "plotcraft/core/PlotPos.h"
#include "plotcraft/database/DataBase.h"
#include "plugin/MyPlugin.h"
#include <string>
#include <thread>
#include <unordered_map>


using string = std::string;
using ll::chrono_literals::operator""_tick;

// 辅助表，用于实现自定义Event
namespace pt {
std::unordered_map<string, plo::core::PlotPos> mIsInPlot; // 玩家是否在地皮中

bool has(string const& realName) { return mIsInPlot.find(realName) != mIsInPlot.end(); }
void set(string realName, plo::core::PlotPos pos) { mIsInPlot[realName] = pos; }

plo::core::PlotPos get(string const& realName) {
    auto it = mIsInPlot.find(realName);
    if (it == mIsInPlot.end()) return plo::core::PlotPos{};
    return it->second;
}
} // namespace pt


// Global variables
ll::schedule::GameTickScheduler mTickScheduler;            // Tick调度
ll::event::ListenerPtr          mPlayerJoinEventListener;  // 玩家加入事件监听器
ll::event::ListenerPtr          mSpawningMobEventListener; // 生物出生事件监听器

namespace plo::event {


void registerEventListener() {
    mTickScheduler.add<ll::schedule::RepeatTask>(4_tick, []() {
        Level& lv = *ll::service::getLevel();
        lv.forEachPlayer([](Player& p) {
            if (p.getDimension().mName != "plot") return true; // 不是同一维度
            if (p.isLoading() || p.isSimulated() || p.isSimulatedPlayer()) return true;
            TextPacket pkt = TextPacket();
            pkt.mType      = TextPacketType::Tip;

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

            // Tip3:
            // PLUGIN_TITLE | 地皮世界
            // 输入: /plo 打开地皮菜单

            auto& bus     = ll::event::EventBus::getInstance();
            auto  plotPos = core::PlotPos(p.getPosition());
            if (plotPos.isValid()) {
                if (auto _pos = pt::get(p.getRealName()); _pos != plotPos) {
                    // 玩家进入地皮
                    bus.publish(PlayerEnterPlot(plotPos, &p)); // 玩家进入地皮，当前位置有效，使用当前位置
                    pt::set(p.getRealName(), plotPos);         // 更新玩家位置
                    p.sendMessage("[Debug] 进入地皮: " + plotPos.toDebug());
                }
                // 获取数据库实例
                auto& pdb  = database::PlotDB::getInstance();
                auto& impl = pdb.getImpl();

                database::Plot plot;

                auto tryGetPlot = pdb.getCached(pdb.hash(plotPos.toString()), database::PlotDB::CacheType::Plot);
                if (tryGetPlot.has_value()) plot = std::get<database::Plot>(*tryGetPlot); // 缓存
                else {
                    auto tryGetPlotDB = impl.getPlot(plotPos.toString()); // 数据库
                    if (tryGetPlotDB.has_value()) plot = *tryGetPlotDB;
                }

                if (plot.mPlotOwner.isEmpty()) {
                    // Tip2
                    pkt.mMessage = fmt::format(
                        "地皮: {0}\n主人: 无主  |  价格: {1}\n输入：/plo buy 购买",
                        plotPos.toDebug(),
                        config::cfg.func.buyPlotPrice
                    );
                } else {
                    // Tip1
                    auto& ndb  = database::PlayerNameDB::getInstance();
                    auto  sale = impl.getSale(plotPos.toString());
                    if (sale.has_value()) {
                        // 玩家出售此地皮
                        pkt.mMessage = fmt::format(
                            "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}",
                            plotPos.toDebug(),
                            ndb.getPlayerName(plot.mPlotOwner),
                            plot.mPlotName,
                            "§a✔§r",
                            sale->mPrice
                        );
                    } else {
                        // 玩家不出售此地皮
                        pkt.mMessage = fmt::format(
                            "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}",
                            plotPos.toDebug(),
                            ndb.getPlayerName(plot.mPlotOwner),
                            plot.mPlotName,
                            "§c✘§r",
                            "null"
                        );
                    }
                }

            } else {
                if (auto _pos2 = pt::get(p.getRealName()); _pos2 != plotPos) {
                    // 玩家离开地皮
                    bus.publish(PlayerLeavePlot(_pos2, &p)); // 玩家离开地皮，当前位置无效，使用上次位置
                    pt::set(p.getRealName(), plotPos);       // 更新玩家位置
                    p.sendMessage("[Debug] 离开地皮: " + _pos2.toDebug());
                }
                // Tip3
                pkt.mMessage = fmt::format("{0} | 地皮世界\n输入: /plo 打开地皮菜单", PLUGIN_TITLE);
            }

            // sendPacket
            p.sendNetworkPacket(pkt);
            return true;
        });
    });


    // Listen Minecraft events
    auto& bus = ll::event::EventBus::getInstance();

    mPlayerJoinEventListener = bus.emplaceListener<ll::event::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& e) {
        database::PlayerNameDB::getInstance().insertPlayer(e.self());
    });

    mSpawningMobEventListener = bus.emplaceListener<ll::event::SpawningMobEvent>([](ll::event::SpawningMobEvent& e) {
        if (e.blockSource().getDimension().mName != "plot") return false; // 拦截地皮世界生物生成
        return true;
    });
}


void unRegisterEventListener() {
    mTickScheduler.clear();

    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(mPlayerJoinEventListener);
    bus.removeListener(mSpawningMobEventListener);
}


} // namespace plo::event