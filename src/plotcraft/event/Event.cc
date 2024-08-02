#include "Event.h"
#include "fmt/format.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/Event.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/enums/BlockUpdateFlag.h"
#include "mc/enums/GameType.h"
#include "mc/enums/TextPacketType.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/UpdateBlockPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/HitResult.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/core/CoreUtils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>


#include "plotcraft/event/hook/SculkBlockGrowthEvent.h"
#include "plotcraft/event/hook/SculkSpreadEvent.h"


#ifdef DEBUG
#define debugger(...) std::cout << "[Debug] " << __VA_ARGS__ << std::endl;
#else
#define debugger(...) ((void)0)
#endif


using string = std::string;
using ll::chrono_literals::operator""_tick;
using PlotPermission = plo::data::PlotPermission;

class CustomEventHelper {
public:
    //               player           PlotPos     Dimension
    std::unordered_map<string, std::pair<plo::PlotPos, int>> mPlayerPos; // 玩家位置缓存

    bool has(string const& uid) { return mPlayerPos.find(uid) != mPlayerPos.end(); }

    void set(string uid, plo::PlotPos pps, int dim) { mPlayerPos[uid] = std::make_pair(pps, dim); }

    std::pair<plo::PlotPos, int> get(string const& uid) {
        auto it = mPlayerPos.find(uid);
        if (it == mPlayerPos.end()) return std::make_pair(plo::PlotPos{}, -1);
        return it->second;
    }
} helper;


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
ll::event::ListenerPtr          mSculkSpreadEventListener;         // 幽匿脉络蔓延
ll::event::ListenerPtr          mSculkBlockGrowthEventListener;    // 幽匿方块生长(幽匿[尖啸/感测]体)
ll::event::ListenerPtr          mPlayerUseItemEventListener;       // 玩家使用物品

ll::event::ListenerPtr mPlayerLeavePlotEventListener; // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEventListener; // 玩家进入地皮

namespace plo::event {

using namespace core_utils;

void buildTipMessage(Player& p, PlotPos const& pps, PlayerNameDB* ndb, PlotBDStorage* pdb) {
    try {
        PlotMetadataPtr plot = pdb->getPlot(pps.getPlotID());
        if (plot == nullptr) plot = PlotMetadata::make(pps.getPlotID(), pps.x, pps.z);

        TextPacket pkt = TextPacket();
        pkt.mType      = TextPacketType::Tip;
        if (pps.isValid()) {
            auto owner = plot->getPlotOwner();
            // clang-format off
            pkt.mMessage = fmt::format(
                "地皮: {0}\n主人: {1}  |  名称: {2}\n出售: {3}  |  价格: {4}{5}",
                pps.toDebug(),
                owner.empty() ? "无主" : ndb->getPlayerName(owner),
                plot->getPlotName(),
                owner.empty() ? "§a✔§r" : plot->isSale() ? "§a✔§r" : "§c✘§r",
                owner.empty() ? config::cfg.plotWorld.buyPlotPrice : plot->isSale() ? plot->getSalePrice() : 0,
                owner.empty() ? "\n输入：/plo plot 打开购买菜单" : ""
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


bool registerEventListener() {
    auto* bus = &ll::event::EventBus::getInstance();
    auto* pdb = &data::PlotBDStorage::getInstance();
    auto* ndb = &data::PlayerNameDB::getInstance();

    // Tick scheduler => 处理自定义事件
    mTickScheduler.add<ll::schedule::RepeatTask>(4_tick, [bus, pdb, ndb]() {
        auto lv = ll::service::getLevel();
        if (!lv) return; // nullptr

        lv->forEachPlayer([bus, pdb, ndb](Player& p) {
            if (p.isSimulatedPlayer() || p.isLoading()) return true; // skip simulated player

            int const     playerDimid = p.getDimensionId();
            int const     plotDimid   = getPlotDimensionId();
            auto const    name        = p.getRealName();
            auto const    pair        = helper.get(name);
            PlotPos const pps{p.getPosition()};

            if (playerDimid != plotDimid) {
                // 玩家通过传送离开地皮维度
                // 地皮维度 => 其它维度  触发
                // 其它维度 => 地皮维度  忽略
                // 其它维度 => 其它维度  忽略
                if (pair.second != -1 && pair.second != playerDimid && pair.second == plotDimid) {
                    debugger("离开地皮（维度）: " << pps.toDebug());
                    bus->publish(PlayerLeavePlot(pair.first, &p));
                    helper.set(name, pps, playerDimid);
                }
                return true;
            }

            if (pdb->getPlayerSetting(p.getUuid().asString()).showPlotTip) buildTipMessage(p, pps, ndb, pdb);
            bool const valid = pps.isValid();

            // Leave
            if ((!valid && pair.first != pps) || (valid && pair.first != pps)) {
                debugger("离开地皮（移动）: " << pair.first.toDebug());
                bus->publish(PlayerLeavePlot(pair.first, &p)); // use last position
            }
            // Join
            if (valid && pair.first != pps) {
                debugger("进入地皮: " << pps.toDebug());
                bus->publish(PlayerEnterPlot(pps, &p));
            }
            helper.set(name, pps, playerDimid);
            return true;
        });
    });

    // My events
    if (config::cfg.plotWorld.inPlotCanFly) {
        mPlayerEnterPlotEventListener = bus->emplaceListener<PlayerEnterPlot>([pdb](PlayerEnterPlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;

            auto const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            auto pps   = PlotPos(pl->getPosition());
            auto level = pdb->getPlayerPermission(pl->getUuid().asString(), pps.toString(), true);

            if (level == PlotPermission::Owner || level == PlotPermission::Shared) {
                pl->setAbility(::AbilitiesIndex::MayFly, true);
                debugger("赋予飞行权限");
            }
        });

        mPlayerLeavePlotEventListener = bus->emplaceListener<PlayerLeavePlot>([](PlayerLeavePlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;

            auto const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            if (pl->canUseAbility(::AbilitiesIndex::MayFly)) {
                pl->setAbility(::AbilitiesIndex::MayFly, false);
                debugger("撤销飞行权限");
            }
        });
    }


    // Minecraft events
    mPlayerJoinEventListener =
        bus->emplaceListener<ll::event::PlayerJoinEvent>([ndb, pdb](ll::event::PlayerJoinEvent& e) {
            if (e.self().isSimulatedPlayer()) return true; // skip simulated player
            ndb->insertPlayer(e.self());
            EconomyQueue::getInstance().transfer(e.self());
            pdb->initPlayerSetting(e.self().getUuid().asString());
            return true;
        });


    mPlayerDestroyBlockEventListener =
        bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>([pdb](ll::event::PlayerDestroyBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true; // 被破坏的方块不在地皮世界

            auto const& blockPos = ev.pos();
            auto        plotPos  = PlotPos(blockPos);
            auto const  lv       = pdb->getPlayerPermission(player.getUuid().asString(), plotPos.toString());

            debugger("破坏方块: " << blockPos.toString() << ", 权限: " << std::to_string(static_cast<int>(lv)));

            if (lv == PlotPermission::Admin) return true; // 放行管理员

            auto const meta = pdb->getPlot(plotPos.getPlotID());
            if (meta) {
                bool const  valid  = plotPos.isValid();
                bool const  border = plotPos.isPosOnBorder(blockPos);
                auto const& tab    = meta->getPermissionTableConst();

                if (!valid || border) ev.cancel(); // 破坏道路 / 边框
                if (valid && !border && lv == PlotPermission::None && !tab.canDestroyBlock)
                    ev.cancel(); // 访客破坏地皮内方块
            } else ev.cancel();  // 破坏未分配的地皮方块
            return true;
        });

    mPlayerPlaceingBlockEventListener =
        bus->emplaceListener<ll::event::PlayerPlacingBlockEvent>([pdb](ll::event::PlayerPlacingBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& blockPos = mc::face2Pos(ev.pos(), ev.face()); // 计算实际放置位置
            auto        pps      = PlotPos(blockPos);
            auto const  lv       = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger("放置方块: " << blockPos.toString() << ", 权限: " << std::to_string(static_cast<int>(lv)));

            if (lv == PlotPermission::Admin) return true;

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                bool const  valid  = pps.isValid();
                bool const  border = pps.isPosOnBorder(blockPos);
                auto const& tab    = meta->getPermissionTableConst();

                if (!valid || border) ev.cancel();
                if (valid && !border && lv == PlotPermission::None && !tab.canPlaceBlock) ev.cancel();
            } else ev.cancel();
            return true;
        });

    mPlayerUseItemOnEventListener =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& vec3  = e.clickPos();
            auto        pps   = PlotPos(vec3);
            auto const  lv    = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());
            bool const  valid = pps.isValid();

            debugger(
                "使用物品: " << e.item().getTypeName() << ", 位置: " << vec3.toString()
                             << ", 权限: " << std::to_string(static_cast<int>(lv))
            );

            if (utils::some(config::cfg.plotWorld.eventListener.onUseItemOnWhiteList, e.item().getTypeName()))
                return true;                                        // 白名单物品
            if (lv == PlotPermission::Admin || !valid) return true; // 管理员或地皮外

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                bool const  border = pps.isPosOnBorder(vec3);
                auto const& tab    = meta->getPermissionTableConst();

                if (border) e.cancel();
                if (valid && !border && lv == PlotPermission::None && !tab.canUseItemOn) e.cancel();
            } else e.cancel();
            return true;
        });

    mFireSpreadEventListener = bus->emplaceListener<ll::event::FireSpreadEvent>([pdb](ll::event::FireSpreadEvent& e) {
        auto const& pos = e.pos();
        auto        pps = PlotPos(pos);
        if (pps.isValid()) {
            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                if (!meta->getPermissionTableConst().canFireSpread) e.cancel();
            }
        } else e.cancel(); // 地皮外
        return true;
    });

    mPlayerAttackEventListener =
        bus->emplaceListener<ll::event::PlayerAttackEvent>([pdb](ll::event::PlayerAttackEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& pos = e.target().getPosition();
            auto        pps = PlotPos(pos);
            auto const  lv  = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger(
                "玩家攻击: " << e.target().getEntityLocNameString() << ", 位置: " << pos.toString()
                             << ", 权限: " << std::to_string(static_cast<int>(lv))
            );

            bool const valid = pps.isValid();
            if (lv == PlotPermission::Admin || !valid) return true; // 管理员或地皮外

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta && lv == PlotPermission::None && !meta->getPermissionTableConst().canAttack) e.cancel();
            return true;
        });

    mPlayerPickUpItemEventListener =
        bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([pdb](ll::event::PlayerPickUpItemEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& pos = e.itemActor().getPosition();
            auto        pps = PlotPos(pos);
            auto const  lv  = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger(
                "玩家捡起物品: " << e.itemActor().getEntityLocNameString() << ", 位置: " << pos.toString()
                                 << ", 权限: " << std::to_string(static_cast<int>(lv))
            );

            bool const valid = pps.isValid();
            if (lv == PlotPermission::Admin || !valid) return true; // 管理员或地皮外

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                if (valid && lv == PlotPermission::None && !meta->getPermissionTableConst().canPickupItem) e.cancel();
            }
            return true;
        });

    mPlayerInteractBlockEventListener =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;
            auto const& pos = e.blockPos(); // 交互的方块位置
            auto        pps = PlotPos(pos);
            auto const  lv  = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger("玩家交互方块: " << pos.toString() << ", 权限: " << std::to_string(static_cast<int>(lv)));

            if (lv == PlotPermission::Admin || !pps.isValid()) return true; // 管理员或地皮外

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                if (lv == PlotPermission::None && !meta->getPermissionTableConst().canInteractBlock) e.cancel();
            }
            return true;
        });

    mPlayerUseItemEventListener =
        bus->emplaceListener<ll::event::PlayerUseItemEvent>([](ll::event::PlayerUseItemEvent& ev) {
            if (ev.self().getDimensionId() != getPlotDimensionId()) return true;
            auto& player = ev.self();

            auto val = player.traceRay(5.5f, false, true, [&](BlockSource const&, Block const& bl, bool) {
                // if (!bl.isSolid()) return false;            // 非固体方块
                if (bl.getMaterial().isLiquid()) return false; // 液体方块
                return true;
            });

            auto const&     item = ev.item();
            BlockPos const& pos  = val.mBlockPos;
            Block const&    bl   = player.getDimensionBlockSource().getBlock(pos);

            debugger(
                "玩家使用物品: " << item.getTypeName() << ", 位置: " << pos.toString() << ", 方块: " << bl.getTypeName()
            );

            auto       pps    = PlotPos(pos);
            bool const border = pps.isPosOnBorder(pos);

            if (border) {
                ev.cancel();
                UpdateBlockPacket(
                    pos,
                    (uint)UpdateBlockPacket::BlockLayer::Extra,
                    bl.getBlockItemId(),
                    (uchar)BlockUpdateFlag::All
                )
                    .sendTo(player); // 防刁民在边框放水，然后客户端不更新
                return true;
            };
            return true;
        });


    // 可开关事件监听器
    if (!config::cfg.plotWorld.spawnMob) {
        mSpawningMobEventListener =
            bus->emplaceListener<ll::event::SpawningMobEvent>([](ll::event::SpawningMobEvent& e) {
                if (e.blockSource().getDimensionId() == getPlotDimensionId()) e.cancel(); // 拦截地皮世界生物生成
                return true;
            });
    }

    if (config::cfg.plotWorld.eventListener.onSculkSpreadListener) {
        mSculkSpreadEventListener = bus->emplaceListener<hook::SculkSpreadEvent>([](hook::SculkSpreadEvent& ev) {
            auto bs = ev.getBlockSource();
            if (!bs.has_value()) return true;
            if (bs->getDimensionId() == getPlotDimensionId()) ev.cancel(); // 地皮世界
            return true;
        });
    }

    if (config::cfg.plotWorld.eventListener.onSculkBlockGrowthListener) {
        mSculkBlockGrowthEventListener =
            bus->emplaceListener<hook::SculkBlockGrowthEvent>([](hook::SculkBlockGrowthEvent& ev) {
                auto sou = ev.getSource();
                if (sou)
                    if (sou->getDimensionId() == getPlotDimensionId()) ev.cancel(); // 地皮世界
                return true;
            });
    }
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
    bus.removeListener(mSculkSpreadEventListener);
    bus.removeListener(mSculkBlockGrowthEventListener);
    bus.removeListener(mPlayerUseItemEventListener);

    return true;
}


} // namespace plo::event