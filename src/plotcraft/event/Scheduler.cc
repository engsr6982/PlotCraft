#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "mc/deps/core/utility/optional_ref.h"
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
#include "plotcraft/math/PlotCross.h"
#include "plotcraft/math/PlotPos.h"
#include "plotcraft/math/PlotRoad.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>
#include <unordered_map>


namespace plot::event {

void SendPlotTip(Player& player, PlotPos const& pps, PlayerNameDB* ndb, PlotDBStorage* db) {
    try {
        PlotMetadataPtr plot = db->getPlot(pps.getPlotID());

        TextPacket pkt = TextPacket();
        pkt.mType      = TextPacketType::Tip;

        if (pps.isValid()) {
            bool const noValue = plot == nullptr;
            bool const noOwner = noValue ? true : plot->getPlotOwner().empty();
            bool const saleing = !noValue ? plot->isSale() : false;
            // clang-format off
            pkt.mMessage = fmt::format(
                "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}{5}",
                pps.toString(),
                noOwner ? "无主" : ndb->getPlayerName(plot->getPlotOwner()),
                noOwner ? "未命名" : plot->getPlotName(),
                noOwner ? "§a✔§r" :saleing ? "§a✔§r" : "§c✘§r",
                noOwner ? Config::cfg.plotWorld.buyPlotPrice : saleing ? plot->getSalePrice() : 0,
                noOwner ? "\n输入：/plot buy 打开购买菜单" : ""
            );
            // clang-format on

        } else {
            // Tip3
#if defined(DEBUG)
            pkt.mMessage = fmt::format(
                "{0} | 地皮世界\n输入: /plot 打开地皮菜单\n\nRoad: {1}\nCross: {2}",
                PLUGIN_TITLE,
                PlotRoad(player.getPosition()).toString(),
                PlotCross(player.getPosition()).toString()
            );
#else
            pkt.mMessage = fmt::format("{0} | 地皮世界\n输入: /plot 打开地皮菜单", PLUGIN_TITLE);
#endif
        }

        pkt.sendTo(player);
    } catch (...) {
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error("Fail in {}\nunknown exception", __FUNCTION__);
    }
}


namespace EventHelper {

std::unordered_map<UUIDm, int>     mDimidMap; // 玩家维度缓存
std::unordered_map<UUIDm, PlotPos> mPlotMap;  // 玩家所在地皮缓存

} // namespace EventHelper


void _SetupPlotEventScheduler() {
    auto* bus = &ll::event::EventBus::getInstance();
    auto* db  = &data::PlotDBStorage::getInstance();
    auto* ndb = &data::PlayerNameDB::getInstance();

    ll::coro::keepThis([bus, db, ndb]() -> ll::coro::CoroTask<> {
        while (ll::getGamingStatus() == ll::GamingStatus::Running) {
            co_await 5_tick;
            ll::service::getLevel()->forEachPlayer([bus, db, ndb](Player& pl) {
                auto& uuid = pl.getUuid();

                // 获取当前位置信息
                auto const& curPos  = pl.getPosition();
                auto const  curDim  = pl.getDimensionId();
                auto const  curPlot = PlotPos(curPos);

                // 获取上一次位置信息
                auto& lastDim  = EventHelper::mDimidMap[uuid];
                auto& lastPlot = EventHelper::mPlotMap[uuid];

                int const plotDimid = getPlotWorldDimensionId();

                // 维度变化
                if (curDim != lastDim) {
                    // 当前不在地皮维度 & 上一次在地皮维度 & 上一次在地皮内
                    if (curDim != plotDimid && lastDim == plotDimid && lastPlot.isValid()) {
                        bus->publish(PlayerLeavePlot{lastPlot, &pl}); // 玩家离开地皮
                    }
                    // 当前在地皮维度 & 上一次不在地皮维度 & 当前在地皮内
                    else if (curDim == plotDimid && lastDim != plotDimid && curPlot.isValid()) {
                        bus->publish(PlayerEnterPlot{curPlot, &pl}); // 玩家进入地皮
                    }
                    lastDim  = curDim;  // 更新维度缓存
                    lastPlot = curPlot; // 更新地皮缓存
                    return true;
                }

                if (curDim != plotDimid) return true; // 不在地皮维度

                if (db->getPlayerSetting(uuid.asString()).showPlotTip) {
                    SendPlotTip(pl, curPlot, ndb, db);
                }

                // 地皮变化
                // 1. 离开地皮
                // 2. 进入地皮
                if (curPlot != lastPlot) {
                    // 上一次在地皮内 & 当前不在地皮内
                    if (lastPlot.isValid() && !curPlot.isValid()) {
                        bus->publish(PlayerLeavePlot{lastPlot, &pl}); // 玩家离开地皮
                    }
                    // 上一次不在地皮内 & 当前在地皮内
                    else if (!lastPlot.isValid() && curPlot.isValid()) {
                        bus->publish(PlayerEnterPlot{curPlot, &pl}); // 玩家进入地皮
                    }
                    lastPlot = curPlot; // 更新地皮缓存
                }

                return true;
            });
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
}

} // namespace plot::event