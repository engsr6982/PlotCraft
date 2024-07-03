#include "Event.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/enums/GameType.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/core/CoreUtils.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include <unordered_map>


using string = std::string;
using ll::chrono_literals::operator""_tick;
using PlotPermission = plo::database::PlotPermission;

class CustomEventHelper {
public:
    //               player           PlotPos     Dimension
    std::unordered_map<UUID, std::pair<plo::PlotPos, int>> mPlayerPos;

    bool has(UUID const& uid) { return mPlayerPos.find(uid) != mPlayerPos.end(); }

    void set(UUID uid, plo::PlotPos pps, int dim) { mPlayerPos[uid] = std::make_pair(pps, dim); }

    std::pair<plo::PlotPos, int> get(UUID const& uid) {
        auto it = mPlayerPos.find(uid);
        if (it == mPlayerPos.end()) return std::make_pair(plo::PlotPos{}, -1);
        return it->second;
    }
} pt;


// Global variables
ll::schedule::GameTickScheduler mTickScheduler;                    // Tick调度
ll::event::ListenerPtr          mPlayerJoinEventListener;          // 玩家进入服务器
ll::event::ListenerPtr          mSpawningMobEventListener;         // 生物尝试生成
ll::event::ListenerPtr          mPlayerDestroyBlockEventListener;  // 玩家尝试破坏方块
ll::event::ListenerPtr          mPlayerPlaceingBlockEventListener; // 玩家尝试放置方块
ll::event::ListenerPtr          mPlayerUseItemOnEventListener;     // 玩家对方块使用物品（点击右键）
ll::event::ListenerPtr          mFireSpreadEventListener;          // 火焰蔓延
ll::event::ListenerPtr          mPlayerAttackEventListener;        // 玩家攻击实体
ll::event::ListenerPtr          mPlayerPickUpItemEventListener;    // 玩家捡起物品
ll::event::ListenerPtr          mPlayerInteractBlockEventListener; // 方块接受玩家互动

ll::event::ListenerPtr mPlayerLeavePlotEventListener; // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEventListener; // 玩家进入地皮

namespace plo::event {

using namespace core_utils;

bool registerEventListener() {
    auto* bus  = &ll::event::EventBus::getInstance();
    auto* pdb  = &database::PlotDB::getInstance();
    auto* impl = &pdb->getImpl();
    auto* ndb  = &database::PlayerNameDB::getInstance();

    // Tick scheduler => 处理自定义事件
    mTickScheduler.add<ll::schedule::RepeatTask>(4_tick, [bus, impl, pdb, ndb]() {
        auto lv = ll::service::getLevel();
        if (!lv) return; // nullptr

        lv->forEachPlayer([bus, impl, pdb, ndb](Player& p) {
            if (p.isSimulatedPlayer()) return true; // skip simulated player

            int     dimid = p.getDimensionId().id;
            PlotPos pps{p.getPosition()};

            if (dimid != getPlotDimensionId()) {
                auto pair = pt.get(p.getUuid());
                if (pair.second != -1 && pair.second != dimid) {
                    // 玩家通过传送离开地皮维度
                    bus->publish(PlayerLeavePlot(pair.first, &p));
                    pt.set(p.getUuid(), pps, dimid);
                }
                return true;
            }


            Plot plot{};
            if (pdb->hasCached(pps.getPlotID())) plot = *pdb->getCached(pps.getPlotID());

            TextPacket pkt = TextPacket();
            pkt.mType      = TextPacketType::Tip;

            if (pps.isValid()) {
                if (pt.get(p.getUuid()).first != pps) { // join
                    bus->publish(PlayerEnterPlot(pps, &p));
                    pt.set(p.getUuid(), pps, dimid);
                }
                if (plot.mPlotOwner.isEmpty()) {
                    // Tip2
                    pkt.mMessage = fmt::format(
                        "地皮: {0}\n主人: 无主  |  价格: {1}\n输入：/plo plot 打开购买菜单",
                        pps.toDebug(),
                        config::cfg.plotWorld.buyPlotPrice
                    );
                } else {
                    // Tip1
                    auto sale = impl->getSale(pps.toString());
                    if (sale.has_value()) {
                        // 玩家出售此地皮
                        pkt.mMessage = fmt::format(
                            "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}",
                            pps.toDebug(),
                            ndb->getPlayerName(plot.mPlotOwner),
                            plot.mPlotName,
                            "§a✔§r",
                            sale->mPrice
                        );
                    } else {
                        // 玩家不出售此地皮
                        pkt.mMessage = fmt::format(
                            "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}",
                            pps.toDebug(),
                            ndb->getPlayerName(plot.mPlotOwner),
                            plot.mPlotName,
                            "§c✘§r",
                            "null"
                        );
                    }
                }
            } else {
                if (auto pair = pt.get(p.getUuid()); pair.first != pps) {
                    bus->publish(PlayerLeavePlot(pair.first, &p)); // use last position
                    pt.set(p.getUuid(), pps, dimid);
                }
                // Tip3
                pkt.mMessage = fmt::format("{0} | 地皮世界\n输入: /plo 打开地皮菜单", PLUGIN_TITLE);
            }

            p.sendNetworkPacket(pkt);
            return true;
        });
    });


    // Minecraft events
    mPlayerJoinEventListener = bus->emplaceListener<ll::event::PlayerJoinEvent>([ndb](ll::event::PlayerJoinEvent& e) {
        if (e.self().isSimulatedPlayer()) return true; // skip simulated player
        ndb->insertPlayer(e.self());
        EconomyQueue::getInstance().transfer(e.self());
        return true;
    });

    if (!config::cfg.plotWorld.spawnMob) {
        mSpawningMobEventListener =
            bus->emplaceListener<ll::event::SpawningMobEvent>([](ll::event::SpawningMobEvent& e) {
                if (e.blockSource().getDimensionId() == getPlotDimensionId()) e.cancel(); // 拦截地皮世界生物生成
                return true;
            });
    }

    mPlayerDestroyBlockEventListener =
        bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>([pdb](ll::event::PlayerDestroyBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true; // 被破坏的方块不在地皮世界

            auto pos   = e.pos();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

#ifdef DEBUG
            player.sendMessage(
                "[Debug] 破坏方块: " + pos.toString() + ", 权限: " + std::to_string(static_cast<int>(level))
            );
#endif

            if (!pps.isValid() && level != PlotPermission::Admin) e.cancel(); // 非管理员破坏道路

            if (pps.isValid() && pps.isPosOnBorder(pos) && level != PlotPermission::Admin)
                e.cancel(); // 非管理员破坏边框

            if (pps.isValid() && !pps.isPosOnBorder(pos) && level == PlotPermission::None)
                e.cancel(); // 访客破坏地皮内方块

            return true;
        });

    mPlayerPlaceingBlockEventListener =
        bus->emplaceListener<ll::event::PlayerPlacingBlockEvent>([pdb](ll::event::PlayerPlacingBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto pos   = e.pos();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

#ifdef DEBUG
            player.sendMessage(
                "[Debug] 放置方块: " + pos.toString() + ", 权限: " + std::to_string(static_cast<int>(level))
            );
#endif

            if (!pps.isValid() && level != PlotPermission::Admin) e.cancel(); // 非管理员在道路放置方块

            if (pps.isValid() && pps.isPosOnBorder(pos) && level != PlotPermission::Admin)
                e.cancel(); // 非管理员在边框放置方块

            if (pps.isValid() && !pps.isPosOnBorder(pos) && level == PlotPermission::None)
                e.cancel(); // 访客在地皮内放置方块

            return true;
        });

    mPlayerUseItemOnEventListener =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto pos   = e.clickPos();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

            // 忽略的物品
            static std::vector<string> ignoreItems = {"minecraft:clock"};
            if (utils::some(ignoreItems, e.item().getTypeName())) return true; // 忽略钟（兼容菜单插件）

#ifdef DEBUG
            player.sendMessage(utils::format(
                "[Debug] 使用物品: {0}, 位置: {1}, 权限: {2}",
                e.item().getTypeName(),
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            if (!pps.isValid() && level != PlotPermission::Admin) e.cancel(); // 非管理员在道路使用物品

            if (pps.isValid() && pps.isPosOnBorder(pos) && level != PlotPermission::Admin)
                e.cancel(); // 非管理员在边框使用物品

            if (pps.isValid() && !pps.isPosOnBorder(pos) && level == PlotPermission::None)
                e.cancel(); // 访客在地皮内使用物品

            return true;
        });

    mFireSpreadEventListener = bus->emplaceListener<ll::event::FireSpreadEvent>([](ll::event::FireSpreadEvent& e) {
        if (!PlotPos{e.pos()}.isValid()) e.cancel(); // 地皮外
        return true;
    });

    mPlayerAttackEventListener =
        bus->emplaceListener<ll::event::PlayerAttackEvent>([pdb](ll::event::PlayerAttackEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto pos   = e.target().getPosition();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

#ifdef DEBUG
            player.sendMessage(utils::format(
                "[Debug] 玩家攻击: {0}, 位置: {1}, 权限: {2}",
                e.target().getEntityLocNameString(),
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            if (!pps.isValid() && level != PlotPermission::Admin) e.cancel();

            if (pps.isValid() && level == PlotPermission::None) e.cancel();

            return true;
        });

    mPlayerPickUpItemEventListener =
        bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([pdb](ll::event::PlayerPickUpItemEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto pos   = e.itemActor().getPosition();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

#ifdef DEBUG
            player.sendMessage(utils::format(
                "[Debug] 玩家捡起物品: {0}, 位置: {1}, 权限: {2}",
                e.itemActor().getEntityLocNameString(),
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            // 地皮内的属于地皮所有者和共享者
            // 地皮外的属于全体玩家
            if (pps.isValid() && level == PlotPermission::None) e.cancel();

            return true;
        });

    mPlayerInteractBlockEventListener =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;
            auto pos   = e.blockPos(); // 交互的方块位置
            auto pps   = PlotPos(pos);
            auto level = pdb->getPermission(player.getUuid(), pps.toString());

#ifdef DEBUG
            player.sendMessage(utils::format(
                "[Debug] 玩家交互方块: 位置: {0}, 权限: {1}",
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            if (!pps.isValid() && level != PlotPermission::Admin) e.cancel();

            if (pps.isValid() && level == PlotPermission::None) e.cancel();

            return true;
        });


    // 监听自己插件的事件
    if (config::cfg.plotWorld.inPlotCanFly) {
        mPlayerEnterPlotEventListener = bus->emplaceListener<PlayerEnterPlot>([pdb](PlayerEnterPlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;

            auto const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            auto pps   = PlotPos(pl->getPosition());
            auto level = pdb->getPermission(pl->getUuid(), pps.toString());

            if (pps.isValid() && level != PlotPermission::None) {
                pl->setAbility(::AbilitiesIndex::MayFly, true);
#ifdef DEBUG
                pl->sendMessage("[Debug] 赋予飞行权限");
#endif
            }
        });

        mPlayerLeavePlotEventListener = bus->emplaceListener<PlayerLeavePlot>([pdb](PlayerLeavePlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;

            auto const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            auto pps   = PlotPos(pl->getPosition());
            auto level = pdb->getPermission(pl->getUuid(), pps.toString());

            if (!pps.isValid() && level != PlotPermission::Admin) {
                pl->setAbility(::AbilitiesIndex::MayFly, false);
#ifdef DEBUG
                pl->sendMessage("[Debug] 撤销飞行权限");
#endif
            }
        });
    }

    // TODO:
    // onMobHurt                生物受伤（包括玩家）
    // onAttackBlock            玩家攻击方块           [lse]
    // onChangeArmorStand       操作盔甲架             [lse]
    // onDropItem               玩家丢出物品           [lse]
    // onUseFrameBlock          操作物品展示框         [lse]
    // onSpawnProjectile        弹射物创建             [lse]
    // onStepOnPressurePlate    生物踩压力板           [lse]
    // onRide                   生物骑乘               [lse]
    // onWitherBossDestroy      凋灵破坏方块           [lse]
    // onPistonTryPush          活塞尝试推动           [lse]
    // onRedStoneUpdate         发生红石更新           [lse]
    // onBlockExplode           发生由方块引起的爆炸    [lse]
    // onLiquidFlow             液体方块流动           [lse]
    return true;
}


bool unRegisterEventListener() {
    mTickScheduler.clear();

    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(mPlayerJoinEventListener);
    bus.removeListener(mSpawningMobEventListener);
    bus.removeListener(mPlayerDestroyBlockEventListener);
    bus.removeListener(mPlayerPlaceingBlockEventListener);
    bus.removeListener(mPlayerUseItemOnEventListener);
    bus.removeListener(mFireSpreadEventListener);
    bus.removeListener(mPlayerAttackEventListener);
    bus.removeListener(mPlayerPickUpItemEventListener);
    bus.removeListener(mPlayerInteractBlockEventListener);
    return true;
}


} // namespace plo::event