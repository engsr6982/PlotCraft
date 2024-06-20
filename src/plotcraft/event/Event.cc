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
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"


using string = std::string;
using ll::chrono_literals::operator""_tick;
using PlotPermission = plo::database::PlotPermission;

namespace pt {
std::unordered_map<string, plo::PlotPos> mIsInPlot; // 玩家是否在地皮中

bool has(string const& realName) { return mIsInPlot.find(realName) != mIsInPlot.end(); }
void set(string realName, plo::PlotPos pos) { mIsInPlot[realName] = pos; }

plo::PlotPos get(string const& realName) {
    auto it = mIsInPlot.find(realName);
    if (it == mIsInPlot.end()) return plo::PlotPos{};
    return it->second;
}
} // namespace pt


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
            auto  plotPos = PlotPos(p.getPosition());
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
                        // TODO：修改命令
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
                // TODO: 修改命令
                pkt.mMessage = fmt::format("{0} | 地皮世界\n输入: /plo 打开地皮菜单", PLUGIN_TITLE);
            }

            // sendPacket
            p.sendNetworkPacket(pkt);
            return true;
        });
    });

    // Listen Minecraft events
    auto& bus                = ll::event::EventBus::getInstance();
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
            auto pos   = e.pos();
            auto pps   = PlotPos(pos);
            auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
            e.self().sendMessage(
                "[Debug] 破坏方块: " + pos.toString() + ", 权限: " + std::to_string(static_cast<int>(level))
            );
#endif

            if (pps.isValid()) {
                // 破坏目标在地皮内
                if (pps.isPosOnBorder(pos)) {
                    if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin 破坏边界方块
                    else utils::sendText<utils::Level::Warn>(e.self(), "请勿破坏地皮边框!");
                } else if (level == PlotPermission::None) e.cancel(); // 拦截 None
            } else {
                // 破坏目标不在地皮内
                if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin
            }
            return true;
        });

    mPlayerPlaceingBlockEventListener =
        bus.emplaceListener<ll::event::PlayerPlacingBlockEvent>([](ll::event::PlayerPlacingBlockEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true;
            auto pos   = e.pos(); // 放置方块的位置
            auto pps   = PlotPos(pos);
            auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

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

    mPlayerUseItemOnEventListener =
        bus.emplaceListener<ll::event::PlayerInteractBlockEvent>([](ll::event::PlayerInteractBlockEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true;
            auto pos   = e.clickPos(); // 点击的位置
            auto pps   = PlotPos(pos); // 获取点击位置的地皮坐标
            auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

            // 忽略的物品
            static std::vector<string> ignoreItems = {"minecraft:clock"};
            if (utils::some(ignoreItems, e.item().getTypeName())) return true; // 忽略钟（兼容菜单插件）

#ifdef DEBUG
            e.self().sendMessage(utils::format(
                "[Debug] 使用物品: {0}, 位置: {1}, 权限: {2}",
                e.item().getTypeName(),
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            if (pps.isValid()) {
                if (level == PlotPermission::None) e.cancel(); // 地皮内, 拦截 None
            } else {
                if (level != PlotPermission::Admin) e.cancel(); // 地皮外, 拦截非 Admin
            }
            return true;
        });

    mFireSpreadEventListener = bus.emplaceListener<ll::event::FireSpreadEvent>([](ll::event::FireSpreadEvent& e) {
        if (e.blockSource().getDimensionId() == getPlotDim()) e.cancel(); // 拦截地皮世界火焰蔓延
        return true;
    });

    mPlayerAttackEventListener = bus.emplaceListener<ll::event::PlayerAttackEvent>([](ll::event::PlayerAttackEvent& e) {
        if (e.self().getDimensionId() != getPlotDim()) return true; // 玩家不在地皮世界
        auto pos   = e.target().getPosition();                      // 攻击的实体位置
        auto pps   = PlotPos(pos);
        auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
        e.self().sendMessage(utils::format(
            "[Debug] 玩家攻击: {0}, 位置: {1}, 权限: {2}",
            e.target().getEntityLocNameString(),
            pos.toString(),
            std::to_string(static_cast<int>(level))
        ));
#endif

        if (pps.isValid()) {
            if (level == PlotPermission::None) e.cancel(); // 拦截 None
        } else {
            if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin
        }
        return true;
    });

    mPlayerPickUpItemEventListener =
        bus.emplaceListener<ll::event::PlayerPickUpItemEvent>([](ll::event::PlayerPickUpItemEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true; // 玩家不在地皮世界
            auto pos   = e.itemActor().getPosition();                   // 要捡起的物品实体位置
            auto pps   = PlotPos(pos);
            auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
            e.self().sendMessage(utils::format(
                "[Debug] 玩家捡起物品: {0}, 位置: {1}, 权限: {2}",
                e.itemActor().getEntityLocNameString(),
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif
            // 拦截规范：
            // 地皮内的属于地皮所有者和共享者
            // 地皮外的属于全体玩家
            if (pps.isValid()) {
                if (level == PlotPermission::None) e.cancel(); // 拦截 None
            }
            return true;
        });

    mPlayerInteractBlockEventListener =
        bus.emplaceListener<ll::event::PlayerInteractBlockEvent>([](ll::event::PlayerInteractBlockEvent& e) {
            if (e.self().getDimensionId() != getPlotDim()) return true;
            auto pos   = e.blockPos(); // 交互的方块位置
            auto pps   = PlotPos(pos);
            auto level = database::PlotDB::getInstance().getPermission(e.self().getUuid(), pps.toString());

#ifdef DEBUG
            e.self().sendMessage(utils::format(
                "[Debug] 玩家交互方块: 位置: {1}, 权限: {2}",
                pos.toString(),
                std::to_string(static_cast<int>(level))
            ));
#endif

            if (pps.isValid()) {
                if (level == PlotPermission::None) e.cancel(); // 拦截 None
            } else {
                if (level != PlotPermission::Admin) e.cancel(); // 拦截非 Admin
            }
            return true;
        });


    // 监听自己插件的事件
    if (config::cfg.func.inPlotCanFly) {
        mPlayerLeavePlotEventListener = bus.emplaceListener<PlayerLeavePlot>([](PlayerLeavePlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;
            if (pl->getPlayerGameType() == GameType::Creative || pl->getPlayerGameType() == GameType::Spectator)
                return; // 不处理创造模式和旁观模式
            auto pps   = PlotPos(pl->getPosition());
            auto level = database::PlotDB::getInstance().getPermission(pl->getUuid(), pps.toString());

            if (!pps.isValid() && level != PlotPermission::Admin) {
                pl->setAbility(::AbilitiesIndex::MayFly, false);
            }
        });
        mPlayerEnterPlotEventListener = bus.emplaceListener<PlayerEnterPlot>([](PlayerEnterPlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;
            if (pl->getPlayerGameType() == GameType::Creative || pl->getPlayerGameType() == GameType::Spectator)
                return; // 不处理创造模式和旁观模式
            auto pps   = PlotPos(pl->getPosition());
            auto level = database::PlotDB::getInstance().getPermission(pl->getUuid(), pps.toString());

            if (pps.isValid() && level != PlotPermission::None) {
                pl->setAbility(::AbilitiesIndex::MayFly, true);
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
    // onFarmLandDecay          耕地退化               [lse]
    // onPistonTryPush          活塞尝试推动           [lse]
    // onRedStoneUpdate         发生红石更新           [lse]
    // onBlockExplode           发生由方块引起的爆炸    [lse]
    // onEntityExplode          发生由实体引起的爆炸    [lse]
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