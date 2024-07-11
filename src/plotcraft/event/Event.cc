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
#include "ll/api/event/world/FireSpreadEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/enums/GameType.h"
#include "mc/enums/TextPacketType.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/core/CoreUtils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>


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
    std::unordered_map<UUID, std::pair<plo::PlotPos, int>> mPlayerPos; // 玩家位置缓存

    bool has(UUID const& uid) { return mPlayerPos.find(uid) != mPlayerPos.end(); }

    void set(UUID uid, plo::PlotPos pps, int dim) { mPlayerPos[uid] = std::make_pair(pps, dim); }

    std::pair<plo::PlotPos, int> get(UUID const& uid) {
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

ll::event::ListenerPtr mPlayerLeavePlotEventListener; // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEventListener; // 玩家进入地皮

namespace plo::event {

using namespace core_utils;

void buildTipMessage(Player& p, PlotPos& pps, PlayerNameDB* ndb, PlotBDStorage* pdb) {
    try {
        std::shared_ptr<PlotMetadata> plot = pdb->getPlot(pps.getPlotID());
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

            int     dimid   = p.getDimensionId();
            int     plotDim = getPlotDimensionId();
            PlotPos pps{p.getPosition()};
            auto    uuid = p.getUuid().asString();
            auto    pair = helper.get(uuid);

            if (dimid != plotDim) {
                // 玩家通过传送离开地皮维度
                // 地皮维度 => 其它维度  触发
                // 其它维度 => 地皮维度  忽略
                // 其它维度 => 其它维度  忽略
                if (pair.second != -1 && pair.second != dimid && pair.second == plotDim) {
                    debugger("离开地皮（维度）: " << pps.toDebug());
                    bus->publish(PlayerLeavePlot(pair.first, &p));
                    helper.set(uuid, pps, dimid);
                }
                return true;
            }

            buildTipMessage(p, pps, ndb, pdb);

            if (pps.isValid()) {
                if (pair.first != pps) { // join
                    debugger("进入地皮: " << pps.toDebug());
                    bus->publish(PlayerEnterPlot(pps, &p));
                }
            } else {
                if (pair.first != pps) {
                    debugger("离开地皮（移动）: " << pair.first.toDebug());
                    bus->publish(PlayerLeavePlot(pair.first, &p)); // use last position
                }
            }
            helper.set(uuid, pps, dimid);
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
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger("破坏方块: " << pos.toString() << ", 权限: " << std::to_string(static_cast<int>(level)));

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
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger("放置方块: " << pos.toString() << ", 权限: " << std::to_string(static_cast<int>(level)));

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
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            // 忽略的物品
            static std::vector<string> ignoreItems = {"minecraft:clock"};
            if (utils::some(ignoreItems, e.item().getTypeName())) return true; // 忽略钟（兼容菜单插件）

            debugger(
                "使用物品: " << e.item().getTypeName() << ", 位置: " << pos.toString()
                             << ", 权限: " << std::to_string(static_cast<int>(level))
            );

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
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger(
                "玩家攻击: " << e.target().getEntityLocNameString() << ", 位置: " << pos.toString()
                             << ", 权限: " << std::to_string(static_cast<int>(level))
            );

            // if (!pps.isValid() && level != PlotPermission::Admin) e.cancel();

            if (pps.isValid() && level == PlotPermission::None) e.cancel();

            return true;
        });

    mPlayerPickUpItemEventListener =
        bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([pdb](ll::event::PlayerPickUpItemEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto pos   = e.itemActor().getPosition();
            auto pps   = PlotPos(pos);
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger(
                "玩家捡起物品: " << e.itemActor().getEntityLocNameString() << ", 位置: " << pos.toString()
                                 << ", 权限: " << std::to_string(static_cast<int>(level))
            );

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
            auto level = pdb->getPlayerPermission(player.getUuid().asString(), pps.toString());

            debugger("玩家交互方块: " << pos.toString() << ", 权限: " << std::to_string(static_cast<int>(level)));

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
            auto level = pdb->getPlayerPermission(pl->getUuid().asString(), pps.toString(), true);

            if (pps.isValid() && (level == PlotPermission::Owner || level == PlotPermission::Shared)) {
                pl->setAbility(::AbilitiesIndex::MayFly, true);
                debugger("赋予飞行权限");
            }
        });

        mPlayerLeavePlotEventListener = bus->emplaceListener<PlayerLeavePlot>([pdb](PlayerLeavePlot& e) {
            auto pl = e.getPlayer();
            if (pl == nullptr) return;

            auto const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            auto pps   = PlotPos(pl->getPosition());
            auto level = pdb->getPlayerPermission(pl->getUuid().asString(), pps.toString(), true);

            if (level == PlotPermission::Owner || level == PlotPermission::Shared) {
                pl->setAbility(::AbilitiesIndex::MayFly, false);
                debugger("撤销飞行权限");
            }
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
    return true;
}


} // namespace plo::event