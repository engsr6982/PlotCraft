#include "Event.h"
#include "TypeNameMap.h"
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

#include "more_events/ActorRideEvent.h"
#include "more_events/ArmorStandSwapItemEvent.h"
#include "more_events/ExplodeEvent.h"
#include "more_events/FarmDecayEvent.h"
#include "more_events/LiquidFlowEvent.h"
#include "more_events/MobHurtEffectEvent.h"
#include "more_events/MossFertilizerEvent.h"
#include "more_events/PistonTryPushEvent.h"
#include "more_events/PlayerAttackBlockEvent.h"
#include "more_events/PlayerDropItemEvent.h"
#include "more_events/PlayerUseItemFrameEvent.h"
#include "more_events/PressurePlateTriggerEvent.h"
#include "more_events/ProjectileSpawnEvent.h"
#include "more_events/RedstoneUpdateEvent.h"
#include "more_events/SculkCatalystAbsorbExperienceEvent.h"
#include "more_events/WitherDestroyBlockEvent.h"


using PlotPermission = plot::data::PlotPermission;


// Global variables
ll::event::ListenerPtr mPlayerJoinEvent;                    // 玩家进入服务器
ll::event::ListenerPtr mSpawningMobEvent;                   // 生物尝试生成
ll::event::ListenerPtr mPlayerDestroyBlockEvent;            // 玩家尝试破坏方块
ll::event::ListenerPtr mPlayerPlaceingBlockEvent;           // 玩家尝试放置方块
ll::event::ListenerPtr mPlayerUseItemOnEvent;               // 玩家对方块使用物品（点击右键）
ll::event::ListenerPtr mFireSpreadEvent;                    // 火焰蔓延
ll::event::ListenerPtr mPlayerAttackEntityEvent;            // 玩家攻击实体
ll::event::ListenerPtr mPlayerPickUpItemEvent;              // 玩家捡起物品
ll::event::ListenerPtr mPlayerInteractBlockEvent;           // 方块接受玩家互动
ll::event::ListenerPtr mPlayerLeavePlotEvent;               // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEvent;               // 玩家进入地皮
ll::event::ListenerPtr mPlayerUseItemEvent;                 // 玩家使用物品
ll::event::ListenerPtr mArmorStandSwapItemEvent;            // 玩家交换盔甲架物品 (more_events)
ll::event::ListenerPtr mPlayerAttackBlockEvent;             // 玩家攻击方块 (more_events)
ll::event::ListenerPtr mPlayerDropItemEvent;                // 玩家丢弃物品 (more_events)
ll::event::ListenerPtr mActorRideEvent;                     // 实体骑乘 (more_events)
ll::event::ListenerPtr mExplodeEvent;                       // 爆炸 (more_events)
ll::event::ListenerPtr mFarmDecayEvent;                     // 农田退化 (more_events)
ll::event::ListenerPtr mMobHurtEffectEvent;                 // 实体受伤效果 (more_events)
ll::event::ListenerPtr mPistonTryPushEvent;                 // 活塞尝试推动方块 (more_events)
ll::event::ListenerPtr mPlayerUseItemFrameEvent;            // 玩家使用物品展示框 (more_events)
ll::event::ListenerPtr mPressurePlateTriggerEvent;          // 压力板触发 (more_events)
ll::event::ListenerPtr mProjectileSpawnEvent;               // 投掷物生成 (more_events)
ll::event::ListenerPtr mRedstoneUpdateEvent;                // 红石更新 (more_events)
ll::event::ListenerPtr mWitherDestroyBlockEvent;            // 凋零破坏方块 (more_events)
ll::event::ListenerPtr mMossFertilizerEvent;                // 苔藓施肥 (more_events)
ll::event::ListenerPtr mLiquidFlowEvent;                    // 流体流动 (more_events)
ll::event::ListenerPtr mSculkCatalystAbsorbExperienceEvent; // 幽匿催发体吸收经验 (more_events)


namespace plot::event {

bool PreCheck(PlotMetadataPtr const& pm, UUIDs const& uuid, bool ignoreAdmin = false) {
    if (!ignoreAdmin && PlotDBStorage::getInstance().isAdmin(uuid)) {
        return true; // 放行管理员
    } else if (pm && pm->getPlayerInThisPlotPermission(uuid) != PlotPermission::None) {
        return true; // 放行地皮主人、共享者
    }
    return false;
}

bool registerEventListener() {
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* db     = &data::PlotDBStorage::getInstance();
    auto* ndb    = &data::PlayerNameDB::getInstance();
    auto* logger = &my_plugin::MyPlugin::getInstance().getSelf().getLogger();

    extern void _SetupPlotEventScheduler();
    _SetupPlotEventScheduler(); // 初始化地皮事件调度器

    // My events
    if (Config::cfg.plotWorld.inPlotCanFly) {
        mPlayerEnterPlotEvent = bus->emplaceListener<PlayerEnterPlot>([db, logger](PlayerEnterPlot& ev) {
            Player* pl = ev.getPlayer();
            if (!pl) {
                return;
            }

            ::GameType const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            auto pps   = PlotPos(pl->getPosition());
            auto level = db->getPlayerPermission(pl->getUuid().asString(), pps.toString(), true);

            if (level == PlotPermission::Owner || level == PlotPermission::Shared) {
                pl->setAbility(::AbilitiesIndex::MayFly, true);
                logger->debug("玩家 {} 获得了飞行权限", pl->getRealName());
            }
        });

        mPlayerLeavePlotEvent = bus->emplaceListener<PlayerLeavePlot>([logger](PlayerLeavePlot& ev) {
            Player* pl = ev.getPlayer();
            if (!pl) {
                return;
            }

            ::GameType const gamemode = pl->getPlayerGameType();
            if (gamemode == GameType::Creative || gamemode == GameType::Spectator) return; // 不处理创造模式和旁观模式

            if (pl->canUseAbility(::AbilitiesIndex::MayFly)) {
                pl->setAbility(::AbilitiesIndex::MayFly, false);
                logger->debug("玩家 {} 的飞行权限已被撤销", pl->getRealName());
            }
        });
    }

    // Minecraft events
    mPlayerJoinEvent = bus->emplaceListener<ll::event::PlayerJoinEvent>([ndb, db](ll::event::PlayerJoinEvent& e) {
        if (e.self().isSimulatedPlayer()) return true; // skip simulated player
        ndb->insertPlayer(e.self());
        db->initPlayerSetting(e.self().getUuid().asString());
        return true;
    });

    mPlayerDestroyBlockEvent =
        bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>([db, logger](ll::event::PlayerDestroyBlockEvent& ev) {
            ServerPlayer& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return; // 被破坏的方块不在地皮世界

            BlockPos const& blockPos = ev.pos();
            PlotPos         pps      = PlotPos(blockPos);

            logger->debug("[DestroyBlock]: {}", blockPos.toString());

            PlotMetadataPtr meta = db->getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) {
                return;
            }

            if (meta) {
                auto const& tab = meta->getPermissionTableConst();

                if (pps.isValid() && !pps.isPosOnBorder(blockPos) && tab.allowDestroy) {
                    return; // 玩家有权限破坏地皮内方块
                }
            }

            ev.cancel();
        });

    mPlayerPlaceingBlockEvent =
        bus->emplaceListener<ll::event::PlayerPlacingBlockEvent>([db, logger](ll::event::PlayerPlacingBlockEvent& ev) {
            ServerPlayer& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            BlockPos blockPos = mc::face2Pos(ev.pos(), ev.face()); // 计算实际放置位置
            auto     pps      = PlotPos(blockPos);

            logger->debug("[PlaceingBlock]: {}", blockPos.toString());

            auto meta = db->getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) {
                return;
            }

            if (meta) {
                auto const& tab = meta->getPermissionTableConst();

                if (pps.isValid() && !pps.isPosOnBorder(blockPos) && tab.allowPlace) return;
            }

            ev.cancel();
        });

    mPlayerUseItemOnEvent =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([db,
                                                                   logger](ll::event::PlayerInteractBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            auto& vec3 = ev.clickPos();
            auto  pps  = PlotPos(vec3);

            logger->debug("[UseItemOn]: Item: {}, Pos: {}", ev.item().getTypeName(), vec3.toString());

            if (!pps.isValid()) return; // 不在地皮上

            auto const meta = db->getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) {
                return;
            }

            auto const& bt = ev.block()->getTypeName();
            auto const& it = ev.item().getTypeName();
            if (!TypeNameMap::UseItemOnMap.contains(bt) && !TypeNameMap::UseItemOnMap.contains(it)) {
                return;
            }

            if (meta) {
                // clang-format off
                auto const& tab = meta->getPermissionTableConst();
                if (it.ends_with("bucket") && tab.useBucket) return ;    // 各种桶
                if (it.ends_with("axe") && tab.allowAxePeeled) return ; // 斧头给木头去皮
                if (it.ends_with("hoe") && tab.useHoe) return ;         // 锄头耕地
                if (it.ends_with("_shovel") && tab.useShovel) return ;  // 锹铲除草径
                if (it == "minecraft:skull" && tab.allowPlace) return ;           // 放置头颅
                if (it == "minecraft:banner" && tab.allowPlace) return ;          // 放置旗帜
                if (it == "minecraft:glow_ink_sac" && tab.allowPlace) return ;    // 发光墨囊给木牌上色
                if (it == "minecraft:end_crystal" && tab.allowPlace) return ;     // 末地水晶
                if (it == "minecraft:ender_eye" && tab.allowPlace) return ;       // 放置末影之眼
                if (it == "minecraft:flint_and_steel" && tab.useFiregen) return ; // 使用打火石
                if (it == "minecraft:bone_meal" && tab.useBoneMeal) return ;      // 使用骨粉
                if (it == "minecraft:minecart"&& tab.allowPlace) return ; // 放置矿车
                if (it == "minecraft:armor_stand"&& tab.allowPlace) return ; // 放置矿车

                if (bt.ends_with("button") && tab.useButton) return ;       // 各种按钮
                if (bt == "minecraft:dragon_egg" && tab.allowAttackDragonEgg) return ; // 右键龙蛋
                if (bt == "minecraft:bed" && tab.useBed) return ;                      // 床
                if ((bt == "minecraft:chest" || bt == "minecraft:trapped_chest") && tab.allowOpenChest) return ; // 箱子&陷阱箱
                if (bt == "minecraft:crafting_table" && tab.useCraftingTable) return ; // 工作台
                if ((bt == "minecraft:campfire" || bt == "minecraft:soul_campfire") && tab.useCampfire) return ; // 营火（烧烤）
                if (bt == "minecraft:composter" && tab.useComposter) return ; // 堆肥桶（放置肥料）
                if (bt.ends_with("shulker_box") && tab.useShulkerBox) return ;  // 潜影盒
                if (bt == "minecraft:noteblock" && tab.useNoteBlock) return ; // 音符盒（调音）
                if (bt == "minecraft:jukebox" && tab.useJukebox) return ;     // 唱片机（放置/取出唱片）
                if (bt == "minecraft:bell" && tab.useBell) return ;           // 钟（敲钟）
                if ((bt == "minecraft:daylight_detector_inverted" || bt == "minecraft:daylight_detector") && tab.useDaylightDetector) return ; // 光线传感器（切换日夜模式）
                if (bt == "minecraft:lectern" && tab.useLectern) return ;                // 讲台
                if (bt == "minecraft:cauldron" && tab.useCauldron) return ;              // 炼药锅
                if (bt == "minecraft:lever" && tab.useLever) return ;                    // 拉杆
                if (bt == "minecraft:respawn_anchor" && tab.useRespawnAnchor) return ;   // 重生锚（充能）
                if (bt.ends_with("_door") && tab.useDoor) return ;            // 各种门
                if (bt.ends_with("fence_gate") && tab.useFenceGate) return ;  // 各种栏栅门
                if (bt.ends_with("trapdoor") && tab.useTrapdoor) return ;     // 各种活板门
                if (bt == "minecraft:flower_pot" && tab.editFlowerPot) return ;          // 花盆
                if (bt.ends_with("_sign") && tab.editSign) return ; // 编辑告示牌
                // clang-format on
            }

            ev.cancel();
        });

    mFireSpreadEvent = bus->emplaceListener<ll::event::FireSpreadEvent>([db](ll::event::FireSpreadEvent& ev) {
        auto const& pos  = ev.pos();
        auto        pps  = PlotPos(pos);
        auto const  meta = db->getPlot(pps.getPlotID());

        if (meta) {
            if (pps.isValid() && meta->getPermissionTableConst().allowFireSpread) return true;
        }

        ev.cancel();
        return true;
    });

    mPlayerAttackEntityEvent =
        bus->emplaceListener<ll::event::PlayerAttackEvent>([db, logger](ll::event::PlayerAttackEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            auto const& pos = ev.target().getPosition();
            auto        pps = PlotPos(pos);

            logger->debug("[AttackEntity] Mob: {}, Pos: {}", ev.target().getTypeName(), pos.toString());

            if (!pps.isValid()) return;

            auto meta = db->getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) {
                return;
            }

            auto const& et = ev.target().getTypeName();
            if (meta) {
                auto const& tab = meta->getPermissionTableConst();
                if (et == "minecraft:ender_crystal" && tab.allowAttackEnderCrystal) return;     // 末影水晶
                if (et == "minecraft:armor_stand" && tab.allowDestroyArmorStand) return;        // 盔甲架
                if (tab.allowAttackPlayer && ev.target().isPlayer()) return;                    // 玩家
                if (tab.allowAttackAnimal && TypeNameMap::AnimalEntityMap.contains(et)) return; // 动物
                if (tab.allowAttackMob && !TypeNameMap::AnimalEntityMap.contains(et)) return;   // 怪物
            }

            ev.cancel();
        });

    mPlayerPickUpItemEvent =
        bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([db, logger](ll::event::PlayerPickUpItemEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            auto const& pos = ev.itemActor().getPosition();
            auto        pps = PlotPos(pos);

            logger->debug("[PickUpItem] Item:{}  Pos: {}", ev.itemActor().getTypeName(), pos.toString());

            if (!pps.isValid()) return;

            auto ptr = db->getPlot(pps.getPlotID());
            if (PreCheck(ptr, player.getUuid().asString())) {
                return;
            }

            if (ptr) {
                auto const& tab = ptr->getPermissionTableConst();
                if (tab.allowPickupItem) return;
            }

            ev.cancel();
        });

    mPlayerInteractBlockEvent =
        bus->emplaceListener<ll::event::PlayerInteractBlockEvent>([db,
                                                                   logger](ll::event::PlayerInteractBlockEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            auto const& pos = ev.blockPos(); // 交互的方块位置
            auto        pps = PlotPos(pos);
            auto const& bn  = ev.block()->getTypeName();

            logger->debug("[InteractBlock]: {}", pos.toString());

            if (!pps.isValid()) return;
            if (!TypeNameMap::InteractBlockMap.contains(bn)) return;

            auto meta = db->getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) return;

            if (meta) {
                auto const& tab = meta->getPermissionTableConst();
                if (bn == "minecraft:cartography_table" && tab.useCartographyTable) return; // 制图台
                if (bn == "minecraft:smithing_table" && tab.useSmithingTable) return;       // 锻造台
                if (bn == "minecraft:brewing_stand" && tab.useBrewingStand) return;         // 酿造台
                if (bn == "minecraft:anvil" && tab.useAnvil) return;                        // 铁砧
                if (bn == "minecraft:grindstone" && tab.useGrindstone) return;              // 磨石
                if (bn == "minecraft:enchanting_table" && tab.useEnchantingTable) return;   // 附魔台
                if (bn == "minecraft:barrel" && tab.useBarrel) return;                      // 桶
                if (bn == "minecraft:beacon" && tab.useBeacon) return;                      // 信标
                if (bn == "minecraft:hopper" && tab.useHopper) return;                      // 漏斗
                if (bn == "minecraft:dropper" && tab.useDropper) return;                    // 投掷器
                if (bn == "minecraft:dispenser" && tab.useDispenser) return;                // 发射器
                if (bn == "minecraft:loom" && tab.useLoom) return;                          // 织布机
                if (bn == "minecraft:stonecutter_block" && tab.useStonecutter) return;      // 切石机
                if (bn.ends_with("blast_furnace") && tab.useBlastFurnace) return;           // 高炉
                if (bn.ends_with("furnace") && tab.useFurnace) return;                      // 熔炉
                if (bn.ends_with("smoker") && tab.useSmoker) return;                        // 烟熏炉
            }

            ev.cancel();
        });

    mPlayerUseItemEvent =
        bus->emplaceListener<ll::event::PlayerUseItemEvent>([logger](ll::event::PlayerUseItemEvent& ev) {
            if (ev.self().getDimensionId() != getPlotWorldDimensionId()) return;
            if (!ev.item().getTypeName().ends_with("bucket")) return;

            auto& player = ev.self();
            auto  val    = player.traceRay(5.5f, false, true, [&](BlockSource const&, Block const& bl, bool) {
                // if (!bl.isSolid()) return false;            // 非固体方块
                if (bl.getMaterial().isLiquid()) return false; // 液体方块
                return true;
            });

            BlockPos const&  pos  = val.mBlockPos;
            ItemStack const& item = ev.item();
            Block const&     bl   = player.getDimensionBlockSource().getBlock(pos);

            logger->debug("[UseItem]: {}, Pos: {}, Block: {}", item.getTypeName(), pos.toString(), bl.getTypeName());

            auto pps = PlotPos(pos);
            if (pps.isValid() && pps.isPosOnBorder(pos)) {
                ev.cancel();
                UpdateBlockPacket(
                    pos,
                    (uint)UpdateBlockPacket::BlockLayer::Extra,
                    bl.getBlockItemId(),
                    (uchar)BlockUpdateFlag::All
                )
                    .sendTo(player); // 防刁民在边框放水，导致客户端不更新
            };
        });

    mPlayerAttackBlockEvent =
        bus->emplaceListener<more_events::PlayerAttackBlockEvent>([logger](more_events::PlayerAttackBlockEvent& ev) {
            optional_ref<Player> pl = ev.getPlayer();
            if (!pl.has_value()) return;
            Player& player = pl.value();

            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[AttackBlock]: Pos: {}", ev.getPos().toString());

            auto pps = PlotPos(ev.getPos());
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) return;

            if (meta) {
                auto const& bl = player.getDimensionBlockSourceConst().getBlock(ev.getPos()).getTypeName();
                if (meta->getPermissionTableConst().allowAttackDragonEgg && bl == "minecraft:dragon_egg") return;
            }

            ev.cancel();
        });

    mArmorStandSwapItemEvent =
        bus->emplaceListener<more_events::ArmorStandSwapItemEvent>([logger](more_events::ArmorStandSwapItemEvent& ev) {
            Player& player = ev.getPlayer();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[ArmorStandSwapItem]: executed");

            auto pps = PlotPos(player.getPosition());
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) return;

            if (meta) {
                if (meta->getPermissionTableConst().useArmorStand) return;
            }

            ev.cancel();
        });

    mPlayerDropItemEvent =
        bus->emplaceListener<more_events::PlayerDropItemEvent>([logger](more_events::PlayerDropItemEvent& ev) {
            Player& player = ev.getPlayer();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[PlayerDropItem]: executed");

            auto pps = PlotPos(player.getPosition());
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (PreCheck(meta, player.getUuid().asString())) return;

            if (meta) {
                if (meta->getPermissionTableConst().allowDropItem) return;
            }

            ev.cancel();
        });

    // 可开关事件（作用于地皮世界）
    mSpawningMobEvent =
        bus->emplaceListener<ll::event::SpawningMobEvent>([/* logger */](ll::event::SpawningMobEvent& ev) {
            if (ev.blockSource().getDimensionId() != getPlotWorldDimensionId()) return;

            // logger->debug("[SpawningMob]: {}", ev.identifier().getFullName());

            if (Config::cfg.plotWorld.spawnMob) return;

            ev.cancel();
        });

    return true;
}


bool unRegisterEventListener() {
    auto& bus = ll::event::EventBus::getInstance();

    bus.removeListener(mPlayerDestroyBlockEvent);
    bus.removeListener(mPlayerPlaceingBlockEvent);
    bus.removeListener(mPlayerUseItemOnEvent);
    bus.removeListener(mFireSpreadEvent);
    bus.removeListener(mPlayerAttackEntityEvent);
    bus.removeListener(mPlayerPickUpItemEvent);
    bus.removeListener(mPlayerInteractBlockEvent);
    bus.removeListener(mPlayerUseItemEvent);

    bus.removeListener(mArmorStandSwapItemEvent);
    bus.removeListener(mPlayerAttackBlockEvent);
    bus.removeListener(mPlayerDropItemEvent);
    bus.removeListener(mActorRideEvent);
    bus.removeListener(mExplodeEvent);
    bus.removeListener(mFarmDecayEvent);
    bus.removeListener(mMobHurtEffectEvent);
    bus.removeListener(mPistonTryPushEvent);
    bus.removeListener(mPlayerUseItemFrameEvent);
    bus.removeListener(mPressurePlateTriggerEvent);
    bus.removeListener(mProjectileSpawnEvent);
    bus.removeListener(mRedstoneUpdateEvent);
    bus.removeListener(mWitherDestroyBlockEvent);

    return true;
}


} // namespace plot::event