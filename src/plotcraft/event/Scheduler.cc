#include "Scheduler.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/Level.h"
#include "plotcraft/Config.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/math/PlotPos.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>


#include "plotcraft/utils/Debugger.h"


ll::schedule::GameTickScheduler mTickScheduler; // Tick调度

using ll::chrono_literals::operator""_tick;


class CustomEventHelper {
public:
    //               player           PlotPos     Dimension
    std::unordered_map<string, std::pair<plo::PlotPos, int>> mPlayerPos; // 玩家位置缓存

    bool has(string const& uid) { return mPlayerPos.find(uid) != mPlayerPos.end(); }

    void set(string uid, plo::PlotPos pps, int dim) { mPlayerPos[uid] = std::make_pair(pps, dim); }

    std::pair<plo::PlotPos, int> const& get(string const& uid) {
        auto it = mPlayerPos.find(uid);
        if (it == mPlayerPos.end()) {
            mPlayerPos[uid] = std::make_pair(plo::PlotPos{}, -1);
            return mPlayerPos[uid];
        }
        return it->second;
    }
} helper;


namespace plo::event {

void buildTipMessage(Player& p, PlotPos const& pps, PlayerNameDB* ndb, PlotDBStorage* pdb) {
    try {
        PlotMetadataPtr plot = pdb->getPlot(pps.getPlotID());
        if (plot == nullptr) plot = PlotMetadata::make(pps.getPlotID(), pps.mX, pps.mZ);

        TextPacket pkt = TextPacket();
        pkt.mType      = TextPacketType::Tip;
        if (pps.isValid()) {
            auto owner = plot->getPlotOwner();
            // clang-format off
            pkt.mMessage = fmt::format(
                "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}{5}",
                pps.toString(),
                owner.empty() ? "无主" : ndb->getPlayerName(owner),
                plot->getPlotName(),
                owner.empty() ? "§a✔§r" : plot->isSale() ? "§a✔§r" : "§c✘§r",
                owner.empty() ? Config::cfg.plotWorld.buyPlotPrice : plot->isSale() ? plot->getSalePrice() : 0,
                owner.empty() ? "\n输入：/plo buy 打开购买菜单" : ""
            );
            // clang-format on
        } else pkt.mMessage = fmt::format("{0} | 地皮世界\n输入: /plo 打开地皮菜单", PLUGIN_TITLE); // Tip3

        p.sendNetworkPacket(pkt);
    } catch (std::exception const& e) {
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Fail in {}\nstd::exception: {}",
            __FUNCTION__,
            e.what()
        );
    } catch (...) {
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error("Fail in {}\nunknown exception", __FUNCTION__);
    }
}


void initPlotEventScheduler() {
    auto* bus = &ll::event::EventBus::getInstance();
    auto* pdb = &data::PlotDBStorage::getInstance();
    auto* ndb = &data::PlayerNameDB::getInstance();

    mTickScheduler.add<ll::schedule::RepeatTask>(4_tick, [bus, pdb, ndb]() {
        auto lv = ll::service::getLevel();
        if (!lv) return; // nullptr

        lv->forEachPlayer([bus, pdb, ndb](Player& p) {
            if (p.isSimulatedPlayer() || p.isLoading()) return true; // skip simulated player

            int const     playerDimid = p.getDimensionId();
            int const     plotDimid   = getPlotWorldDimensionId();
            auto const    name        = p.getRealName();
            auto const&   pair        = helper.get(name);
            PlotPos const pps{p.getPosition()};

            if (playerDimid != plotDimid) {
                // 玩家通过传送离开地皮维度
                // 地皮维度 => 其它维度  触发
                // 其它维度 => 地皮维度  忽略
                // 其它维度 => 其它维度  忽略
                if (pair.second != -1 && pair.second != playerDimid && pair.second == plotDimid) {
                    debugger("离开地皮（维度）: " << pps.toString());
                    bus->publish(PlayerLeavePlot(pair.first, &p));
                    helper.set(name, pps, playerDimid);
                }
                return true;
            }

            if (pdb->getPlayerSetting(p.getUuid().asString()).showPlotTip) buildTipMessage(p, pps, ndb, pdb);
            bool const valid = pps.isValid();

            // Leave
            if ((!valid && pair.first != pps) || (valid && pair.first != pps)) {
                debugger("离开地皮（移动）: " << pair.first.toString());
                bus->publish(PlayerLeavePlot(pair.first, &p)); // use last position
            }
            // Join
            if (valid && pair.first != pps) {
                debugger("进入地皮: " << pps.toString());
                bus->publish(PlayerEnterPlot(pps, &p));
            }
            helper.set(name, pps, playerDimid);
            return true;
        });
    });
}

} // namespace plo::event