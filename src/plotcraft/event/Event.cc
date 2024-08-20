#include "Event.h"
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
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/enums/BlockUpdateFlag.h"
#include "mc/enums/GameType.h"
#include "mc/network/packet/UpdateBlockPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/HitResult.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/core/PlotPos.h"
#include "plotcraft/core/Utils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>

#include "EventHook.h"
#include "RuntimeMap.h"
#include "Scheduler.h"
#include "plotcraft/utils/Debugger.h"


#include "plotcraft/event/hook/SculkBlockGrowthEvent.h"
#include "plotcraft/event/hook/SculkSpreadEvent.h"

using string         = std::string;
using PlotPermission = plo::data::PlotPermission;


// Global variables
ll::event::ListenerPtr mPlayerJoinEvent;          // 玩家进入服务器
ll::event::ListenerPtr mSpawningMobEvent;         // 生物尝试生成
ll::event::ListenerPtr mPlayerDestroyBlockEvent;  // 玩家尝试破坏方块
ll::event::ListenerPtr mPlayerPlaceingBlockEvent; // 玩家尝试放置方块
ll::event::ListenerPtr mPlayerUseItemOnEvent;     // 玩家对方块使用物品（点击右键）
ll::event::ListenerPtr mFireSpreadEvent;          // 火焰蔓延
ll::event::ListenerPtr mPlayerAttackEntityEvent;  // 玩家攻击实体
ll::event::ListenerPtr mPlayerPickUpItemEvent;    // 玩家捡起物品
ll::event::ListenerPtr mPlayerInteractBlockEvent; // 方块接受玩家互动
ll::event::ListenerPtr mSculkSpreadEvent;         // 幽匿脉络蔓延
ll::event::ListenerPtr mSculkBlockGrowthEvent;    // 幽匿方块生长(幽匿[尖啸/感测]体)
ll::event::ListenerPtr mPlayerUseItemEvent;       // 玩家使用物品
ll::event::ListenerPtr mPlayerLeavePlotEvent;     // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEvent;     // 玩家进入地皮

namespace plo::event {
using namespace core;

bool CheckPerm(PlotDBStorage* pdb, PlotID const& id, UUIDs const& uuid, bool ignoreAdmin) {
    PlotDBStorage* db = pdb ? pdb : &PlotDBStorage::getInstance();
    return db->getPlayerPermission(uuid, id, ignoreAdmin) != PlotPermission::None;
}
bool StringFind(string const& str, string const& sub) { return str.rfind(sub) != string::npos; }


bool registerEventListener() {
    auto* bus = &ll::event::EventBus::getInstance();
    auto* pdb = &data::PlotDBStorage::getInstance();
    auto* ndb = &data::PlayerNameDB::getInstance();

    // My events
    initPlotEventScheduler(); // 初始化地皮事件调度器
    if (config::cfg.plotWorld.inPlotCanFly) {
        mPlayerEnterPlotEvent = bus->emplaceListener<PlayerEnterPlot>([pdb](PlayerEnterPlot& e) {
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

        mPlayerLeavePlotEvent = bus->emplaceListener<PlayerLeavePlot>([](PlayerLeavePlot& e) {
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
    registerHook();

    mPlayerJoinEvent = bus->emplaceListener<ll::event::PlayerJoinEvent>([ndb, pdb](ll::event::PlayerJoinEvent& e) {
        if (e.self().isSimulatedPlayer()) return true; // skip simulated player
        ndb->insertPlayer(e.self());
        EconomyQueue::getInstance().transfer(e.self());
        pdb->initPlayerSetting(e.self().getUuid().asString());
        return true;
    });

    mPlayerDestroyBlockEvent =
        bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>([pdb](ll::event::PlayerDestroyBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true; // 被破坏的方块不在地皮世界

            auto const& blockPos = ev.pos();
            auto        pps      = PlotPos(blockPos);

            debugger("[DestroyBlock]: " << blockPos.toString());

            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString(), false)) return true;

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                auto const& tab = meta->getPermissionTableConst();

                if (pps.isValid() && !pps.isPosOnBorder(blockPos) && tab.allow_destroy)
                    return true; // 玩家有权限破坏地皮内方块
            }

            ev.cancel();
            return true;
        });

    mPlayerPlaceingBlockEvent =
        bus->emplaceListener<ll::event::PlayerPlacingBlockEvent>([pdb](ll::event::PlayerPlacingBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& blockPos = mc::face2Pos(ev.pos(), ev.face()); // 计算实际放置位置
            auto        pps      = PlotPos(blockPos);

            debugger("[PlaceingBlock]: " << blockPos.toString());

            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString())) return true;

            auto const meta = pdb->getPlot(pps.getPlotID());
            if (meta) {
                auto const& tab = meta->getPermissionTableConst();

                if (pps.isValid() && !pps.isPosOnBorder(blockPos) && tab.allow_place) return true;
            }

            ev.cancel();
            return true;
        });

    mPlayerUseItemOnEvent =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& vec3 = e.clickPos();
            auto        pps  = PlotPos(vec3);

            debugger("[UseItemOn]: " << e.item().getTypeName() << ", 位置: " << vec3.toString());

            if (!pps.isValid()) return true; // 不在地皮上
            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString())) return true;

            auto const meta = pdb->getPlot(pps.getPlotID());

            bool        hasMap = false;
            auto const& bt     = e.block()->getTypeName();
            auto const& it     = e.item().getTypeName();
            if (!RuntimeMap::has(MapType::UseItem, bt)) {
                if (!RuntimeMap::has(MapType::UseItem, it)) {
                    return true;
                } else {
                    hasMap = true;
                }
            }

            if (meta) {
                auto const& tab = meta->getPermissionTableConst();
                // clang-format off
                if (hasMap) {
                    if (StringFind(it, "bucket") && tab.use_bucket) return true;   // 各种桶
                    if (StringFind(it, "axe") && tab.allow_place) return true;     // 斧头给木头去皮
                    if (it == "minecraft:skull" && tab.allow_place) return true;          // 放置头颅
                    if (it == "minecraft:banner" && tab.allow_place) return true;         // 放置旗帜
                    if (it == "minecraft:glow_ink_sac" && tab.allow_place) return true;   // 发光墨囊给木牌上色
                    if (it == "minecraft:end_crystal" && tab.allow_place) return true;    // 末地水晶
                    if (it == "minecraft:ender_eye" && tab.allow_place) return true;      // 放置末影之眼
                    if (it == "minecraft:flint_and_steel" && tab.use_firegen) return true;// 使用打火石
                } else {
                    if (StringFind(bt,"button")   && tab.use_button) return true; // 各种按钮
			        if (bt == "minecraft:dragon_egg" && tab.allow_destroy) return true; // 右键龙蛋
			        if (bt == "minecraft:bed" && tab.use_bed) return true; // 床
			        if ((bt == "minecraft:chest" || bt == "minecraft:trapped_chest") && tab.allow_open_chest) return true; // 箱子&陷阱箱
			        if (bt == "minecraft:crafting_table" && tab.use_crafting_table) return true; // 工作台
			        if ((bt == "minecraft:campfire" || bt == "minecraft:soul_campfire") && tab.use_campfire) return true; // 营火（烧烤）
			        if (bt == "minecraft:composter" && tab.use_composter) return true; // 堆肥桶（放置肥料）
			        if ((bt == "minecraft:undyed_shulker_box" || bt == "minecraft:shulker_box") && tab.use_shulker_box) return true; // 潜匿箱
			        if (bt == "minecraft:noteblock" && tab.use_noteblock) return true; // 音符盒（调音）
			        if (bt == "minecraft:jukebox" && tab.use_jukebox) return true; // 唱片机（放置/取出唱片）
			        if (bt == "minecraft:bell" && tab.use_bell) return true; // 钟（敲钟）
			        if ((bt == "minecraft:daylight_detector_inverted" || bt == "minecraft:daylight_detector") && tab.use_daylight_detector) return true; // 光线传感器（切换日夜模式）
			        if (bt == "minecraft:lectern" && tab.use_lectern) return true; // 讲台
			        if (bt == "minecraft:cauldron" && tab.use_cauldron) return true; // 炼药锅
			        if (bt == "minecraft:lever" && tab.use_lever) return true; // 拉杆
			        if (bt == "minecraft:respawn_anchor" && tab.use_respawn_anchor) return true; // 重生锚（充能）
			        if (StringFind(bt,"_door") && tab.use_door) return true; // 各种门
			        if (StringFind(bt , "fence_gate")  && tab.use_fence_gate) return true; // 各种栏栅门
			        if (StringFind(bt,"trapdoor")  && tab.use_trapdoor) return true; // 各种活板门
			        if (bt == "minecraft:flower_pot" && tab.edit_flower_pot) return true; // 花盆
                }
                // clang-format on
            }

            e.cancel();
            return true;
        });

    mFireSpreadEvent = bus->emplaceListener<ll::event::FireSpreadEvent>([pdb](ll::event::FireSpreadEvent& e) {
        auto const& pos  = e.pos();
        auto        pps  = PlotPos(pos);
        auto const  meta = pdb->getPlot(pps.getPlotID());

        if (meta) {
            if (pps.isValid() && meta->getPermissionTableConst().allowFireSpread) return true;
        }

        e.cancel();
        return true;
    });

    mPlayerAttackEntityEvent =
        bus->emplaceListener<ll::event::PlayerAttackEvent>([pdb](ll::event::PlayerAttackEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& pos = e.target().getPosition();
            auto        pps = PlotPos(pos);

            debugger("[AttackEntity]: " << e.target().getTypeName() << ", 位置: " << pos.toString());

            if (!pps.isValid()) return true;
            if (pdb->isAdmin(player.getUuid().asString())) return true;

            auto const& et  = e.target().getTypeName();
            auto const& tab = pdb->getPlot(pps.getPlotID())->getPermissionTableConst();
            if (RuntimeMap::has(MapType::SpecialAttack, et)) {
                if (et == "minecraft:ender_crystal" && tab.allow_destroy) return true; // 末影水晶
                if (et == "minecraft:armor_stand" && tab.allow_destroy) return true;   // 盔甲架
            } else {
                if (tab.allow_attack_player && e.target().isPlayer()) return true;                      // 玩家
                if (tab.allow_attack_animal && RuntimeMap::has(MapType::AnimalEntity, et)) return true; // 动物
                if (tab.allow_attack_mobs && RuntimeMap::has(MapType::MobEntity, et)) return true;      // 怪物
            }

            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString(), true)) return true;

            e.cancel();
            return true;
        });

    mPlayerPickUpItemEvent =
        bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([pdb](ll::event::PlayerPickUpItemEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& pos = e.itemActor().getPosition();
            auto        pps = PlotPos(pos);

            debugger("[PickUpItem]: " << e.itemActor().getTypeName() << ", 位置: " << pos.toString());

            if (!pps.isValid()) return true;
            if (pdb->isAdmin(player.getUuid().asString())) return true;

            auto const& tab = pdb->getPlot(pps.getPlotID())->getPermissionTableConst();
            if (tab.allow_pickupitem) return true;

            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString(), true)) return true;

            e.cancel();
            return true;
        });

    mPlayerInteractBlockEvent =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([pdb](ll::event::PlayerInteractBlockEvent& e) {
            auto& player = e.self();
            if (player.getDimensionId() != getPlotDimensionId()) return true;

            auto const& pos = e.blockPos(); // 交互的方块位置
            auto        pps = PlotPos(pos);

            debugger("[InteractBlock]: " << pos.toString());

            if (!pps.isValid()) return true;
            if (CheckPerm(pdb, pps.getPlotID(), player.getUuid().asString())) return true;

            auto const& tab = pdb->getPlot(pps.getPlotID())->getPermissionTableConst();
            auto const& bn  = e.block()->getTypeName();

            if (bn == "minecraft:cartography_table" && tab.use_cartography_table) return true; // 制图台
            if (bn == "minecraft:smithing_table" && tab.use_smithing_table) return true;       // 锻造台
            if (bn == "minecraft:brewing_stand" && tab.use_brewing_stand) return true;         // 酿造台
            if (bn == "minecraft:anvil" && tab.use_anvil) return true;                         // 铁砧
            if (bn == "minecraft:grindstone" && tab.use_grindstone) return true;               // 磨石
            if (bn == "minecraft:enchanting_table" && tab.use_enchanting_table) return true;   // 附魔台
            if (bn == "minecraft:barrel" && tab.use_barrel) return true;                       // 桶
            if (bn == "minecraft:beacon" && tab.use_beacon) return true;                       // 信标
            if (bn == "minecraft:hopper" && tab.use_hopper) return true;                       // 漏斗
            if (bn == "minecraft:dropper" && tab.use_dropper) return true;                     // 投掷器
            if (bn == "minecraft:dispenser" && tab.use_dispenser) return true;                 // 发射器
            if (bn == "minecraft:loom" && tab.use_loom) return true;                           // 织布机
            if (bn == "minecraft:stonecutter_block" && tab.use_stonecutter) return true;       // 切石机
            if (StringFind(bn, "blast_furnace") && tab.use_blast_furnace) return true;         // 高炉
            if (StringFind(bn, "furnace") && tab.use_furnace) return true;                     // 熔炉
            if (StringFind(bn, "smoker") && tab.use_smoker) return true;                       // 烟熏炉

            e.cancel();
            return true;
        });

    mPlayerUseItemEvent = bus->emplaceListener<ll::event::PlayerUseItemEvent>([](ll::event::PlayerUseItemEvent& ev) {
        if (ev.self().getDimensionId() != getPlotDimensionId()) return true;
        auto& player = ev.self();

        auto const val = player.traceRay(5.5f, false, true, [&](BlockSource const&, Block const& bl, bool) {
            // if (!bl.isSolid()) return false;            // 非固体方块
            if (bl.getMaterial().isLiquid()) return false; // 液体方块
            return true;
        });

        auto const&     item = ev.item();
        BlockPos const& pos  = val.mBlockPos;
        Block const&    bl   = player.getDimensionBlockSource().getBlock(pos);

        debugger("[UseItem]: " << item.getTypeName() << ", 位置: " << pos.toString() << ", 方块: " << bl.getTypeName());

        if (PlotPos(pos).isPosOnBorder(pos)) {
            ev.cancel();
            UpdateBlockPacket(
                pos,
                (uint)UpdateBlockPacket::BlockLayer::Extra,
                bl.getBlockItemId(),
                (uchar)BlockUpdateFlag::All
            )
                .sendTo(player); // 防刁民在边框放水，导致客户端不更新
            return true;
        };
        return true;
    });


    // 可开关事件（作用于地皮世界）
    if (!config::cfg.plotWorld.spawnMob) {
        mSpawningMobEvent = bus->emplaceListener<ll::event::SpawningMobEvent>([](ll::event::SpawningMobEvent& e) {
            if (e.blockSource().getDimensionId() == getPlotDimensionId()) e.cancel(); // 拦截地皮世界生物生成
            return true;
        });
    }

    if (config::cfg.plotWorld.eventListener.onSculkSpreadListener) {
        mSculkSpreadEvent = bus->emplaceListener<hook::SculkSpreadEvent>([](hook::SculkSpreadEvent& ev) {
            auto bs = ev.getBlockSource();
            if (!bs.has_value()) return true;
            if (bs->getDimensionId() == getPlotDimensionId()) ev.cancel(); // 地皮世界
            return true;
        });
    }

    if (config::cfg.plotWorld.eventListener.onSculkBlockGrowthListener) {
        mSculkBlockGrowthEvent = bus->emplaceListener<hook::SculkBlockGrowthEvent>([](hook::SculkBlockGrowthEvent& ev) {
            auto sou = ev.getSource();
            if (sou)
                if (sou->getDimensionId() == getPlotDimensionId()) ev.cancel(); // 地皮世界
            return true;
        });
    }
    return true;
}


bool unRegisterEventListener() {
    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(mPlayerJoinEvent);
    bus.removeListener(mSpawningMobEvent);
    bus.removeListener(mPlayerDestroyBlockEvent);
    bus.removeListener(mPlayerPlaceingBlockEvent);
    bus.removeListener(mPlayerUseItemOnEvent);
    bus.removeListener(mFireSpreadEvent);
    bus.removeListener(mPlayerAttackEntityEvent);
    bus.removeListener(mPlayerPickUpItemEvent);
    bus.removeListener(mPlayerInteractBlockEvent);
    bus.removeListener(mSculkSpreadEvent);
    bus.removeListener(mSculkBlockGrowthEvent);
    bus.removeListener(mPlayerUseItemEvent);

    unregisterHook();

    return true;
}


} // namespace plo::event