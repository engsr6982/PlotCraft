#include "Event.h"
#include "TypeNameMap.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/ListenerBase.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/network/packet/UpdateBlockPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"
#include "mc\world\level\block\components\BlockLiquidDetectionComponent.h"
#include "mc\world\level\chunk\SubChunk.h"
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
#include <functional>
#include <memory>
#include <optional>
#include <string>


#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"

#include "ila/event/minecraft/actor/ActorRideEvent.h"
#include "ila/event/minecraft/actor/ActorTriggerPressurePlateEvent.h"
#include "ila/event/minecraft/actor/ArmorStandSwapItemEvent.h"
#include "ila/event/minecraft/actor/MobHurtEffectEvent.h"
#include "ila/event/minecraft/actor/ProjectileCreateEvent.h"
// #include "ila/event/minecraft/level/SculkCatalystAbsorbExperienceEvent.h"
#include "ila/event/minecraft/player/PlayerAttackBlockEvent.h"
#include "ila/event/minecraft/player/PlayerDropItemEvent.h"
#include "ila/event/minecraft/player/PlayerOperatedItemFrameEvent.h"
#include "ila/event/minecraft/world/ExplosionEvent.h"
#include "ila/event/minecraft/world/FarmDecayEvent.h"
#include "ila/event/minecraft/world/LiquidTryFlowEvent.h"
#include "ila/event/minecraft/world/MossGrowthEvent.h"
#include "ila/event/minecraft/world/PistonPushEvent.h"
#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/event/minecraft/world/SculkBlockGrowthEvent.h"
#include "ila/event/minecraft/world/SculkSpreadEvent.h"
#include "ila/event/minecraft/world/WitherDestroyEvent.h"


using PlotPermission = plot::data::PlotPermission;


// Global variables
ll::event::ListenerPtr mPlayerJoinEvent;           // 玩家进入服务器
ll::event::ListenerPtr mActorHurtEvent;            // 实体受伤
ll::event::ListenerPtr mSpawningMobEvent;          // 生物尝试生成
ll::event::ListenerPtr mPlayerDestroyBlockEvent;   // 玩家尝试破坏方块
ll::event::ListenerPtr mPlayerPlaceingBlockEvent;  // 玩家尝试放置方块
ll::event::ListenerPtr mPlayerUseItemOnEvent;      // 玩家对方块使用物品（点击右键）
ll::event::ListenerPtr mFireSpreadEvent;           // 火焰蔓延
ll::event::ListenerPtr mPlayerAttackEntityEvent;   // 玩家攻击实体
ll::event::ListenerPtr mPlayerPickUpItemEvent;     // 玩家捡起物品
ll::event::ListenerPtr mPlayerInteractBlockEvent;  // 方块接受玩家互动
ll::event::ListenerPtr mPlayerLeavePlotEvent;      // 玩家离开地皮
ll::event::ListenerPtr mPlayerEnterPlotEvent;      // 玩家进入地皮
ll::event::ListenerPtr mPlayerUseItemEvent;        // 玩家使用物品
ll::event::ListenerPtr mArmorStandSwapItemEvent;   // 玩家交换盔甲架物品 (iListenAttentively)
ll::event::ListenerPtr mPlayerAttackBlockEvent;    // 玩家攻击方块 (iListenAttentively)
ll::event::ListenerPtr mPlayerDropItemEvent;       // 玩家丢弃物品 (iListenAttentively)
ll::event::ListenerPtr mActorRideEvent;            // 实体骑乘 (iListenAttentively)
ll::event::ListenerPtr mExplodeEvent;              // 爆炸 (iListenAttentively)
ll::event::ListenerPtr mFarmDecayEvent;            // 农田退化 (iListenAttentively)
ll::event::ListenerPtr mMobHurtEffectEvent;        // 实体受伤效果 (iListenAttentively)
ll::event::ListenerPtr mPistonTryPushEvent;        // 活塞尝试推动方块 (iListenAttentively)
ll::event::ListenerPtr mPlayerUseItemFrameEvent;   // 玩家使用物品展示框 (iListenAttentively)
ll::event::ListenerPtr mPressurePlateTriggerEvent; // 压力板触发 (iListenAttentively)
ll::event::ListenerPtr mProjectileSpawnEvent;      // 投掷物生成 (iListenAttentively)
ll::event::ListenerPtr mRedstoneUpdateEvent;       // 红石更新 (iListenAttentively)
ll::event::ListenerPtr mWitherDestroyBlockEvent;   // 凋零破坏方块 (iListenAttentively)
ll::event::ListenerPtr mMossFertilizerEvent;       // 苔藓施肥 (iListenAttentively)
ll::event::ListenerPtr mLiquidFlowEvent;           // 流体流动 (iListenAttentively)
// ll::event::ListenerPtr mSculkCatalystAbsorbExperienceEvent; // 幽匿催发体吸收经验 (iListenAttentively)
ll::event::ListenerPtr mSculkBlockGrowthEvent; // 幽匿尖啸体生成 (iListenAttentively)
ll::event::ListenerPtr mSculkSpreadEvent;      // 幽匿蔓延 (iListenAttentively)


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

    // LeviLamina's events
    mPlayerJoinEvent = bus->emplaceListener<ll::event::PlayerJoinEvent>([ndb, db](ll::event::PlayerJoinEvent& e) {
        if (e.self().isSimulatedPlayer()) return true; // skip simulated player
        ndb->insertPlayer(e.self());
        db->initPlayerSetting(e.self().getUuid().asString());
        return true;
    });

    mPlayerDestroyBlockEvent =
        bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>([db, logger](ll::event::PlayerDestroyBlockEvent& ev) {
            auto& player = ev.self();
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
            auto& player = ev.self();
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
                if (bl.getMaterial().isLiquid()) return false; // 液体方块
                return true;
            });

            BlockPos const&  pos  = val.mBlock;
            ItemStack const& item = ev.item();
            Block const&     bl   = player.getDimensionBlockSource().getBlock(pos);

            logger->debug("[UseItem]: {}, Pos: {}, Block: {}", item.getTypeName(), pos.toString(), bl.getTypeName());

            auto pps = PlotPos(pos);
            if (pps.isValid() && pps.isPosOnBorder(pos)) {
                ev.cancel();
                static uchar flags = (1 << 0) | (1 << 1); // 0b11 BlockUpdateFlag::All v0.13.5
                UpdateBlockPacket(
                    pos,
                    (uint)SubChunk::BlockLayer::Extra,
                    bl.getBlockItemId(),
                    flags
                )
                    .sendTo(player); // 防刁民在边框放水，导致客户端不更新
            };
        });

    // 可开关事件（作用于地皮世界）
    mSpawningMobEvent =
        bus->emplaceListener<ll::event::SpawningMobEvent>([/* logger */](ll::event::SpawningMobEvent& ev) {
            if (ev.blockSource().getDimensionId() != getPlotWorldDimensionId()) return;

            // logger->debug("[SpawningMob]: {}", ev.identifier().getFullName());

            if (Config::cfg.plotWorld.spawnMob) return;

            ev.cancel();
        });

    mActorHurtEvent = bus->emplaceListener<ll::event::ActorHurtEvent>([db, logger](ll::event::ActorHurtEvent& ev) {
        auto& self = ev.self();
        if (self.getDimensionId() != getPlotWorldDimensionId()) return;

        logger->debug("[ActorHurtEvent] mob: {}", self.getTypeName());

        auto pps = PlotPos(ev.self().getPosition());
        if (!pps.isValid()) return;

        auto meta = db->getPlot(pps.getPlotID());
        if (meta) {
            auto const& et  = self.getTypeName();
            auto const& tab = meta->getPermissionTableConst();
            if (tab.allowAttackPlayer && self.isPlayer()) return;
            if (tab.allowAttackAnimal && TypeNameMap::AnimalEntityMap.contains(et)) return;
            if (tab.allowAttackMob && !TypeNameMap::AnimalEntityMap.contains(et)) return;
        }

        if (self.isPlayer()) {
            auto const pl = self.getWeakEntity().tryUnwrap<Player>();
            if (pl.has_value()) {
                if (PreCheck(meta, pl->getUuid().asString())) return;
            }
        }

        ev.cancel();
    });

    // iLa
    mPlayerAttackBlockEvent =
        bus->emplaceListener<ila::mc::PlayerAttackBlockBeforeEvent>([logger](ila::mc::PlayerAttackBlockBeforeEvent& ev
                                                                    ) {
            optional_ref<Player> pl = ev.self();
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
        bus->emplaceListener<ila::mc::ArmorStandSwapItemBeforeEvent>([logger](ila::mc::ArmorStandSwapItemBeforeEvent& ev
                                                                     ) {
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
        bus->emplaceListener<ila::mc::PlayerDropItemBeforeEvent>([logger](ila::mc::PlayerDropItemBeforeEvent& ev) {
            Player& player = ev.self();
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

    mActorRideEvent = bus->emplaceListener<ila::mc::ActorRideBeforeEvent>([logger](ila::mc::ActorRideBeforeEvent& ev) {
        Actor& passenger = ev.self();
        if (passenger.getDimensionId() != getPlotWorldDimensionId()) return;

        logger->debug("[生物骑乘] executed!");

        if (!passenger.isPlayer()) return;

        auto pps = PlotPos(passenger.getPosition());
        if (!pps.isValid()) return;

        auto const& type = ev.getTarget().getTypeName();
        auto const  meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            auto const& tab = meta->getPermissionTableConst();
            if (type == "minecraft:minecart" || type == "minecraft:boat") {
                if (tab.allowRideTrans) return;
            } else {
                if (tab.allowRideEntity) return;
            }
        }

        if (passenger.isPlayer()) {
            auto pl = passenger.getWeakEntity().tryUnwrap<Player>();
            if (pl.has_value()) {
                if (PreCheck(meta, pl->getUuid().asString())) return;
            }
        }

        return;
    });

    mExplodeEvent = bus->emplaceListener<ila::mc::ExplosionBeforeEvent>([logger](ila::mc::ExplosionBeforeEvent& ev) {
        auto& bs  = ev.blockSource();
        auto& exp = ev.getExplosion();
        auto  pos = BlockPos{exp.mPos};

        if (bs.getDimensionId() != getPlotWorldDimensionId()) return;

        logger->debug("[Explode] pos: {}", pos.toString());

        int   r    = (int)(exp.mRadius + 1.0);
        auto  land = PlotPos::getPlotPosAt(pos, r);
        auto& db   = PlotDBStorage::getInstance();

        for (auto& p : land) {
            if (p.isRadiusOnBorder(pos, r)) {
                ev.cancel(); // 禁止破坏边框
                return;
            }

            auto meta = db.getPlot(p.getPlotID());
            if (meta) {
                if (meta->getPermissionTableConst().allowExplode) {
                    return; // 允许爆炸
                }
            }
        }

        ev.cancel(); // 地皮世界禁止爆炸
    });

    mFarmDecayEvent = bus->emplaceListener<ila::mc::FarmDecayBeforeEvent>([logger](ila::mc::FarmDecayBeforeEvent& ev) {
        auto& region = ev.blockSource();
        if (region.getDimensionId() != getPlotWorldDimensionId()) return;

        logger->debug("[耕地退化] pos: {}", ev.getPos().toString());

        auto pps = PlotPos(ev.getPos());
        if (!pps.isValid()) return;

        auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            if (meta->getPermissionTableConst().allowFarmDecay) return;
        }

        ev.cancel();
    });

    mMobHurtEffectEvent =
        bus->emplaceListener<ila::mc::MobHurtEffectBeforeEvent>([logger](ila::mc::MobHurtEffectBeforeEvent& ev) {
            auto& self = ev.self();
            if (self.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[MobHurt] mob: {}", self.getTypeName());

            auto pps = PlotPos(ev.self().getPosition());
            if (!pps.isValid()) return;

            auto meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (meta) {
                auto const& et  = self.getTypeName();
                auto const& tab = meta->getPermissionTableConst();
                if (tab.allowAttackPlayer && self.isPlayer()) return;
                if (tab.allowAttackAnimal && TypeNameMap::AnimalEntityMap.contains(et)) return;
                if (tab.allowAttackMob && !TypeNameMap::AnimalEntityMap.contains(et)) return;
            }

            if (self.isPlayer()) {
                auto const pl = self.getWeakEntity().tryUnwrap<Player>();
                if (pl.has_value()) {
                    if (PreCheck(meta, pl->getUuid().asString())) return;
                }
            }

            ev.cancel();
        });

    mPistonTryPushEvent =
        bus->emplaceListener<ila::mc::PistonPushBeforeEvent>([logger](ila::mc::PistonPushBeforeEvent& ev) {
            auto& region = ev.blockSource();
            if (region.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[活塞推动方块] 目标: {}", ev.getPushPos().toString());

            auto sou = PlotPos(ev.getPistonPos());
            auto tar = PlotPos(ev.getPushPos());

            if (sou.isValid() && tar.isValid() && sou == tar) {
                if (!sou.isPosOnBorder(ev.getPushPos()) && !tar.isPosOnBorder(ev.getPushPos())) {
                    auto const meta = PlotDBStorage::getInstance().getPlot(sou.getPlotID());
                    if (meta) {
                        if (meta->getPermissionTableConst().allowPistonPush) return;
                    }
                }
            }

            ev.cancel();
        });

    mPlayerUseItemFrameEvent = bus->emplaceListener<ila::mc::PlayerOperatedItemFrameBeforeEvent>(
        [logger](ila::mc::PlayerOperatedItemFrameBeforeEvent& ev) {
            auto& player = ev.self();
            if (player.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[物品展示框] pos: {}", ev.getBlockPos().toString());

            auto pps = PlotPos(ev.getBlockPos());
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (meta) {
                if (meta->getPermissionTableConst().useItemFrame) return;
            }
            if (PreCheck(meta, player.getUuid().asString())) return;

            ev.cancel();
        }
    );

    mPressurePlateTriggerEvent = bus->emplaceListener<ila::mc::ActorTriggerPressurePlateBeforeEvent>(
        [logger](ila::mc::ActorTriggerPressurePlateBeforeEvent& ev) {
            auto& self   = ev.self();
            auto& region = self.getDimensionBlockSource();
            auto& pos    = ev.getPos();

            if (region.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[压力板] pos: {} entity: {}", pos.toString(), self.getTypeName());

            auto pps = PlotPos(pos);
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (meta) {
                if (meta->getPermissionTableConst().usePressurePlate) return;
            }

            if (self.isPlayer()) {
                auto pl = self.getWeakEntity().tryUnwrap<Player>();
                if (pl.has_value()) {
                    if (PreCheck(meta, pl->getUuid().asString())) return;
                }
            }

            ev.cancel();
        }
    );

    mProjectileSpawnEvent =
        bus->emplaceListener<ila::mc::ProjectileCreateBeforeEvent>([logger](ila::mc::ProjectileCreateBeforeEvent& ev) {
            auto& actor = ev.self();
            if (actor.getDimensionId() != getPlotWorldDimensionId()) return;

            auto const& type = actor.getTypeName();

            logger->debug("[弹射物生成] type: {}", type);

            auto pps = PlotPos(actor.getPosition());
            if (!pps.isValid()) return;


            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (meta) {
                auto const& tab = meta->getPermissionTableConst();
                if (type == "minecraft:fishing_hook" && tab.useFishingHook) return;       // 钓鱼竿
                if (type == "minecraft:splash_potion" && tab.allowThrowPotion) return;    // 喷溅药水
                if (type == "minecraft:lingering_potion" && tab.allowThrowPotion) return; // 滞留药水
                if (type == "minecraft:thrown_trident" && tab.allowThrowTrident) return;  // 三叉戟
                if (type == "minecraft:arrow" && tab.allowShoot) return;                  // 箭
                if (type == "minecraft:crossbow" && tab.allowShoot) return;               // 弩射烟花
                if (type == "minecraft:snowball" && tab.allowThrowSnowball) return;       // 雪球
                if (type == "minecraft:ender_pearl" && tab.allowThrowEnderPearl) return;  // 末影珍珠
                if (type == "minecraft:egg" && tab.allowThrowEgg) return;                 // 鸡蛋
            }

            if (actor.isPlayer()) {
                auto pl = actor.getWeakEntity().tryUnwrap<Player>();
                if (pl.has_value()) {
                    if (PreCheck(meta, pl->getUuid().asString())) return;
                }
            }

            ev.cancel();
        });

    mRedstoneUpdateEvent =
        bus->emplaceListener<ila::mc::RedstoneUpdateBeforeEvent>([logger](ila::mc::RedstoneUpdateBeforeEvent& ev) {
            auto& region = ev.blockSource();
            if (region.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[RedstoneUpdate] pos: {}", ev.getPos().toString());

            auto pps = PlotPos(ev.getPos());
            if (!pps.isValid()) return;

            auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
            if (meta) {
                if (meta->getPermissionTableConst().allowRedstoneUpdate) return;
            }

            ev.cancel();
        });

    mWitherDestroyBlockEvent =
        bus->emplaceListener<ila::mc::WitherDestroyBeforeEvent>([logger](ila::mc::WitherDestroyBeforeEvent& ev) {
            auto& region = ev.blockSource();
            if (region.getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[凋零破坏方块] executed!");

            auto& bb   = ev.getBox();
            auto  land = PlotPos::getPlotPosAt(bb.min, bb.max);
            auto& db   = PlotDBStorage::getInstance();

            for (auto const& p : land) {
                if (p.isAABBOnBorder(bb.min, bb.max)) {
                    my_plugin::MyPlugin::getInstance().getSelf().getLogger().warn(
                        "Wither try to destroy block on border of plot {} at {}",
                        p.getPlotID(),
                        bb.toString()
                    );
                    ev.cancel();
                    return; // 禁止破坏边框
                }

                auto meta = db.getPlot(p.getPlotID());
                if (meta) {
                    if (!meta->getPermissionTableConst().allowWitherDestroy) {
                        ev.cancel();
                        return;
                    }
                } else {
                    ev.cancel();
                    return;
                }
            }
        });

    mMossFertilizerEvent =
        bus->emplaceListener<ila::mc::MossGrowthBeforeEvent>([logger, db](ila::mc::MossGrowthBeforeEvent& ev) {
            if (ev.blockSource().getDimensionId() != getPlotWorldDimensionId()) return;

            logger->debug("[MossSpread] {}", ev.getPos().toString());

            auto const& pos = ev.getPos();
            auto        pps = PlotPos(pos);

            auto land = db->getPlot(pps.getPlotID());
            if (!land || !land->getPermissionTableConst().useBoneMeal) {
                ev.cancel();
                return;
            }

            auto lds = PlotPos::getPlotPosAt(pos - 2, pos + 2);
            for (auto const& p : lds) {
                if (p.isRadiusOnBorder(pos, 2)) {
                    ev.cancel();
                    return;
                }
            }
        });

    mLiquidFlowEvent =
        bus->emplaceListener<ila::mc::LiquidTryFlowBeforeEvent>([](ila::mc::LiquidTryFlowBeforeEvent& ev) {
            auto& bs = ev.blockSource();
            if (bs.getDimensionId() != getPlotWorldDimensionId()) return;

            auto& sou = ev.getPos();
            auto  pps = PlotPos(sou);
            if (!pps.isValid()) {
                ev.cancel(); // 禁止在非领地流动
                return;
            }

            if (pps.isPosOnBorder(sou)) {
                ev.cancel(); // 禁止在边框流动
                return;
            }
        });

    // mSculkCatalystAbsorbExperienceEvent = bus->emplaceListener<more_events::SculkCatalystAbsorbExperienceEvent>(
    //     [logger](more_events::SculkCatalystAbsorbExperienceEvent& ev) {
    //         auto& actor  = ev.getDiedActor();
    //         auto& region = actor.getDimensionBlockSource();
    //         if (region.getDimensionId() != getPlotWorldDimensionId()) return;

    //         auto pos = actor.getBlockPosCurrentlyStandingOn(&actor);

    //         logger->debug("[SculkCatalystAbsorbExperience] Pos: {}", pos.toString());

    //         auto pps = PlotPos(pos);
    //         if (!pps.isValid()) {
    //             ev.cancel(); // 禁止在非领地蔓延
    //             return;
    //         }

    //         if (pps.isAABBOnBorder(pos - 9, pos + 9)) {
    //             ev.cancel(); // 禁止在边框蔓延
    //             return;
    //         }
    //     }
    // );

    mSculkBlockGrowthEvent =
        bus->emplaceListener<ila::mc::SculkBlockGrowthBeforeEvent>([db,
                                                                    logger](ila::mc::SculkBlockGrowthBeforeEvent& ev) {
            auto& pos = ev.getPos();
            logger->debug("[SculkBlockGrowth] {}", pos.toString());

            PlotPos pps(pos);
            if (!pps.isValid()) {
                ev.cancel(); // 禁止在非领地蔓延
                return;
            }

            if (pps.isPosOnBorder(pos)) {
                ev.cancel(); // 禁止在边框蔓延
                return;
            }
        });

    mSculkSpreadEvent =
        bus->emplaceListener<ila::mc::SculkSpreadBeforeEvent>([logger](ila::mc::SculkSpreadBeforeEvent& ev) {
            logger->debug("[SculkSpread] {} -> {}", ev.getSelfPos().toString(), ev.getTargetPos().toString());

            PlotPos sou{ev.getSelfPos()};
            PlotPos tar{ev.getTargetPos()};

            if (!sou.isValid() || !tar.isValid()) {
                ev.cancel(); // 道路禁止蔓延
                return;
            }
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
    bus.removeListener(mMossFertilizerEvent);
    bus.removeListener(mLiquidFlowEvent);
    // bus.removeListener(mSculkCatalystAbsorbExperienceEvent);
    return true;
}


} // namespace plot::event