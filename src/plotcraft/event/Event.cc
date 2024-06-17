#include "Event.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

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
ll::schedule::GameTickScheduler mTickScheduler;                    // Tick调度
ll::event::ListenerPtr          mPlayerJoinEventListener;          // 玩家进入服务器
ll::event::ListenerPtr          mSpawningMobEventListener;         // 生物尝试生成
ll::event::ListenerPtr          mPlayerDestroyBlockEventListener;  // 玩家尝试破坏方块
ll::event::ListenerPtr          mPlayerPlaceingBlockEventListener; // 玩家尝试放置方块

namespace plo::event {

DimensionType getPlotDim() { return VanillaDimensions::fromString("plot"); }

bool registerEventListener() {
    // 注册Tick调度(实现Tip、Event等)
    mTickScheduler.add<ll::schedule::RepeatTask>(4_tick, []() {
        auto lv = ll::service::getLevel()->getDimension(getPlotDim());
        if (!lv) return; // 空指针
        lv->forEachPlayer([](Player& p) {
            if (p.getDimensionId() != getPlotDim()) return true; // 不是地皮世界，跳过
            if (p.isLoading() || p.isSimulated() || p.isSimulatedPlayer()) return true;
            TextPacket pkt = TextPacket();
            pkt.mType      = TextPacketType::Tip;
            /*
                §a✔§r
                §c✘§r

                Tip1:
                地皮: (0,1) | (0,0,0) => (16,16,16)
                主人: xxx  |  名称: xxx
                出售: x/√  |  价格: xxx

                Tip2:
                地皮: (0,1) | (0,0,0) => (16,16,16)
                主人: 无主  | 价格: xxx
                输入：/plo buy 购买

                Tip3:
                PLUGIN_TITLE | 地皮世界
                输入: /plo 打开地皮菜单
             */
            auto& bus     = ll::event::EventBus::getInstance();
            auto  plotPos = core::PlotPos(p.getPosition());
            if (plotPos.isValid()) {
                if (auto _pos = pt::get(p.getRealName()); _pos != plotPos) {
                    // 玩家进入地皮
                    bus.publish(PlayerEnterPlot(plotPos, &p)); // 玩家进入地皮，当前位置有效，使用当前位置
                    pt::set(p.getRealName(), plotPos);         // 更新玩家位置
#ifdef DEBUG
                    p.sendMessage("[Debug] 进入地皮: " + plotPos.toDebug());
#endif
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
#ifdef DEBUG
                    p.sendMessage("[Debug] 离开地皮: " + _pos2.toDebug());
#endif
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
        if (e.blockSource().getDimensionId() == getPlotDim()) e.cancel(); // 拦截地皮世界生物生成
        return true;
    });

    mPlayerDestroyBlockEventListener =
        bus.emplaceListener<ll::event::PlayerDestroyBlockEvent>([](ll::event::PlayerDestroyBlockEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true; // 被破坏的方块不在地皮世界
            auto pos = e.pos();
            auto pps = core::PlotPos(pos);

            using PlotPermission = database::PlotPermission;
            auto level           = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
            e.self().sendMessage(
                "[Debug] 破坏方块: " + pos.toString() + ", 权限: " + std::to_string(static_cast<int>(level))
            );
#endif

            if (pps.isValid()) {
                // 破坏目标在地皮内
                if (level == PlotPermission::None) e.cancel(); // 拦截 None
            } else {
                // 破坏目标不在地皮内
                if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin
            }
            return true;
        });

    mPlayerPlaceingBlockEventListener =
        bus.emplaceListener<ll::event::PlayerPlacingBlockEvent>([](ll::event::PlayerPlacingBlockEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true;
            auto pos = e.pos(); // 放置方块的位置
            auto pps = core::PlotPos(pos);

            using PlotPermission = database::PlotPermission;
            auto level           = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
            e.self().sendMessage(
                "[Debug] 放置方块: " + pos.toString() + ", 权限: " + std::to_string(static_cast<int>(level))
            );
#endif

            if (pps.isValid()) {
                if (level == PlotPermission::None) e.cancel(); // 拦截 None
            } else {
                if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin
            }
            return true;
        });

    // TODO:
    // onUseItemOn
    // onAttackBlock
    // onAttackEntity
    // onChangeArmorStand
    // onTakeItem
    // onDropItem
    // onBlockInteracted
    // onUseFrameBlock
    // onSpawnProjectile
    // onMobHurt
    // onStepOnPressurePlate
    // onRide
    // onWitherBossDestroy
    // onFarmLandDecay
    // onPistonTryPush
    // onFireSpread
    // onEat
    // onRedStoneUpdate
    // onStartDestroyBlock
    // onBlockExplode
    // onEntityExplode


    return true;
}


bool unRegisterEventListener() {
    mTickScheduler.clear();

    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(mPlayerJoinEventListener);
    bus.removeListener(mSpawningMobEventListener);
    bus.removeListener(mPlayerDestroyBlockEventListener);
    bus.removeListener(mPlayerPlaceingBlockEventListener);
    return true;
}


} // namespace plo::event