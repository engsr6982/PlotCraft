#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/entity/utilities/ActorDamageCause.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/ActorUniqueID.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/containers/ContainerID.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/scores/ScoreInfo.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <mc/common/wrapper/InteractionResult.h>
#include <mc/entity/WeakEntityRef.h>
#include <mc/entity/components/ProjectileComponent.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/module/VanillaServerGameplayEventListener.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/FishingHook.h>
#include <mc/world/actor/Hopper.h>
#include <mc/world/actor/VanillaActorRendererId.h>
#include <mc/world/actor/boss/WitherBoss.h>
#include <mc/world/actor/item/ItemActor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/containers/models/LevelContainerModel.h>
#include <mc/world/events/EventResult.h>
#include <mc/world/inventory/transaction/ComplexInventoryTransaction.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/CrossbowItem.h>
#include <mc/world/item/ItemInstance.h>
#include <mc/world/item/TridentItem.h>
#include <mc/world/item/registry/ItemStack.h>
#include <mc/world/level/BlockEventCoordinator.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ChangeDimensionRequest.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Spawner.h>
#include <mc/world/level/block/BasePressurePlateBlock.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/ComparatorBlock.h>
#include <mc/world/level/block/DiodeBlock.h>
#include <mc/world/level/block/FarmBlock.h>
#include <mc/world/level/block/ItemFrameBlock.h>
#include <mc/world/level/block/LiquidBlockDynamic.h>
#include <mc/world/level/block/RedStoneWireBlock.h>
#include <mc/world/level/block/RedstoneTorchBlock.h>
#include <mc/world/level/block/RespawnAnchorBlock.h>
#include <mc/world/level/block/actor/BarrelBlockActor.h>
#include <mc/world/level/block/actor/BaseCommandBlock.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/block/actor/ChestBlockActor.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/scores/ServerScoreboard.h>

#include "Event.h"
#include "RuntimeMap.h"
#include "Scheduler.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/core/Utils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/math/PPos.h"
#include "plotcraft/utils/Debugger.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include "plugin/MyPlugin.h"



using string         = std::string;
using PlotPermission = plo::data::PlotPermission;

#define CATCH                                                                                                          \
    catch (std::exception const& e) {                                                                                  \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "[Hook] Catch exception at {}: {}",                                                                        \
            __FUNCTION__,                                                                                              \
            e.what()                                                                                                   \
        );                                                                                                             \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "[Hook] Catch unknown exception at {}",                                                                    \
            __FUNCTION__                                                                                               \
        );                                                                                                             \
    }
#define CATCH_RET(ret)                                                                                                 \
    catch (std::exception const& e) {                                                                                  \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "[Hook] Catch exception at {}: {}",                                                                        \
            __FUNCTION__,                                                                                              \
            e.what()                                                                                                   \
        );                                                                                                             \
        return ret;                                                                                                    \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "[Hook] Catch unknown exception at {}",                                                                    \
            __FUNCTION__                                                                                               \
        );                                                                                                             \
        return ret;                                                                                                    \
    }

#define Normal High

namespace plo::event {
using namespace core;


// 生物受伤
LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ActorDamageSource const& source,
    float                    damage
) {
    try {
        if (this->getDimensionId() != getPlotDimensionId()) return origin(source, damage);

        debugger("[MobHurt] mob: " << this->getTypeName());

        auto pps = PPos(this->getPosition());
        if (!pps.isValid()) return origin(source, damage);

        auto meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            auto const& et  = this->getTypeName();
            auto const& tab = meta->getPermissionTableConst();
            if (tab.allowAttackPlayer && this->isPlayer()) return origin(source, damage);
            if (tab.allowAttackAnimal && RuntimeMap::has(MapType::AnimalEntity, et)) return origin(source, damage);
            if (tab.allowAttackMob && RuntimeMap::has(MapType::MobEntity, et)) return origin(source, damage);
        }

        if (this->isPlayer()) {
            auto const pl = this->getWeakEntity().tryUnwrap<Player>();
            if (pl.has_value()) {
                if (CheckPerm(nullptr, pps.getPlotID(), pl->getUuid().asString())) return origin(source, damage);
            }
        }

        return 0.0f;
    }
    CATCH_RET(0.0f);
}


// 耕地退化
LL_TYPE_INSTANCE_HOOK(
    FarmDecayHook,
    HookPriority::Normal,
    FarmBlock,
    "?transformOnFall@FarmBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@PEAVActor@@M@Z",
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          actor,
    float           fallDistance
) {
    try {
        if (region.getDimensionId() != getPlotDimensionId()) return origin(region, pos, actor, fallDistance);

        debugger("[耕地退化] pos: " << pos.toString());

        auto pps = PPos(pos);
        if (!pps.isValid()) return origin(region, pos, actor, fallDistance);

        auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            if (meta->getPermissionTableConst().allowFarmDecay) return origin(region, pos, actor, fallDistance);
        }
    }
    CATCH;
}


// 玩家操作物品展示框
const auto UseFrameBlockCallback = [](Player& player, BlockPos const& pos) -> bool {
    if (player.getDimensionId() != getPlotDimensionId()) return true;

    debugger("[物品展示框] pos: " << pos.toString());

    auto pps = PPos(pos);
    if (!pps.isValid()) return true;

    auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
    if (meta) {
        if (meta->getPermissionTableConst().useItemFrame) return true;
    }
    if (CheckPerm(nullptr, pps.getPlotID(), player.getUuid().asString())) return true;

    return false;
};
LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    "?use@ItemFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@E@Z",
    bool,
    Player&         player,
    BlockPos const& pos,
    uchar           face
) {
    try {
        if (UseFrameBlockCallback(player, pos)) return origin(player, pos, face);
        return false;
    }
    CATCH_RET(false);
}
LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook2,
    HookPriority::Normal,
    ItemFrameBlock,
    "?attack@ItemFrameBlock@@UEBA_NPEAVPlayer@@AEBVBlockPos@@@Z",
    bool,
    Player*         player,
    BlockPos const& pos
) {
    try {
        if (UseFrameBlockCallback(*player, pos)) return origin(player, pos);
        return false;
    }
    CATCH_RET(false);
}


// 弹射物生成
const auto SpawnProjectileCallback = [](Actor* actor, string const& type) -> bool {
    if (!actor) return true;
    if (actor->getDimensionId() != getPlotDimensionId()) return true;

    debugger("[弹射物生成] type: " << type);

    auto pps = PPos(actor->getPosition());
    if (!pps.isValid()) return true;

    auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
    if (meta) {
        auto const& tab = meta->getPermissionTableConst();
        if (type == "minecraft:fishing_hook" && tab.useFishingHook) return true;       // 钓鱼竿
        if (type == "minecraft:splash_potion" && tab.allowThrowPotion) return true;    // 喷溅药水
        if (type == "minecraft:lingering_potion" && tab.allowThrowPotion) return true; // 滞留药水
        if (type == "minecraft:thrown_trident" && tab.allowThrowTrident) return true;  // 三叉戟
        if (type == "minecraft:arrow" && tab.allowShoot) return true;                  // 箭
        if (type == "minecraft:crossbow" && tab.allowShoot) return true;               // 弩射烟花
        if (type == "minecraft:snowball" && tab.allowThrowSnowball) return true;       // 雪球
        if (type == "minecraft:ender_pearl" && tab.allowThrowEnderPearl) return true;  // 末影珍珠
        if (type == "minecraft:egg" && tab.allowThrowEgg) return true;                 // 鸡蛋
    }

    if (actor->isPlayer()) {
        auto pl = actor->getWeakEntity().tryUnwrap<Player>();
        if (pl.has_value()) {
            if (CheckPerm(nullptr, pps.getPlotID(), pl->getUuid().asString())) return true;
        }
    }

    return false;
};
LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook1,
    HookPriority::Normal,
    Spawner,
    &Spawner::spawnProjectile,
    Actor*,
    BlockSource&                     region,
    ActorDefinitionIdentifier const& id,
    Actor*                           spawner,
    Vec3 const&                      position,
    Vec3 const&                      direction
) {
    try {
        if (!spawner) {
            if (region.getDimensionId() != getPlotDimensionId()) // fix spawner nullptr
                return origin(region, id, spawner, position, direction);
        }
        if (SpawnProjectileCallback(spawner, id.getCanonicalName()))
            return origin(region, id, spawner, position, direction);
        return nullptr;
    }
    CATCH_RET(nullptr);
}
LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook2,
    HookPriority::Normal,
    CrossbowItem,
    &CrossbowItem::_shootFirework,
    void,
    ItemInstance const& projectileInstance,
    Player&             player
) {
    try {
        if (SpawnProjectileCallback(&player, projectileInstance.getTypeName()))
            return origin(projectileInstance, player);
    }
    CATCH;
}
LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook3,
    HookPriority::Normal,
    TridentItem,
    "?releaseUsing@TridentItem@@UEBAXAEAVItemStack@@PEAVPlayer@@H@Z",
    void,
    ItemStack& item,
    Player*    player,
    int        durationLeft
) {
    try {
        if (SpawnProjectileCallback(player, VanillaActorRendererId::trident.getString()))
            return origin(item, player, durationLeft);
    }
    CATCH;
}


// 生物踩踏压力板
LL_TYPE_INSTANCE_HOOK(
    PressurePlateTriggerHook,
    HookPriority::Normal,
    BasePressurePlateBlock,
    "?shouldTriggerEntityInside@BasePressurePlateBlock@@UEBA_NAEAVBlockSource@@AEBVBlockPos@@AEAVActor@@@Z",
    bool,
    BlockSource&    region,
    BlockPos const& pos,
    Actor&          entity
) {
    try {
        if (region.getDimensionId() != getPlotDimensionId()) return origin(region, pos, entity);

        debugger("[压力板] pos: " << pos.toString() << " entity: " << entity.getTypeName());

        auto pps = PPos(pos);
        if (!pps.isValid()) return origin(region, pos, entity);

        auto const meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            if (meta->getPermissionTableConst().usePressurePlate) return origin(region, pos, entity);
        }

        if (entity.isPlayer()) {
            auto pl = entity.getWeakEntity().tryUnwrap<Player>();
            if (pl.has_value()) {
                if (CheckPerm(nullptr, pps.getPlotID(), pl->getUuid().asString())) return origin(region, pos, entity);
            }
        }

        return false;
    }
    CATCH_RET(false);
}


// 生物骑乘
LL_TYPE_INSTANCE_HOOK(
    ActorRideHook,
    HookPriority::Normal,
    Actor,
    "?canAddPassenger@Actor@@UEBA_NAEAV1@@Z",
    bool,
    Actor& passenger
) {
    try {
        if (passenger.getDimensionId() != getPlotDimensionId()) return origin(passenger);

        debugger("[生物骑乘] executed!");

        if (!passenger.isPlayer()) return origin(passenger);

        auto pps = PPos(passenger.getPosition());
        if (!pps.isValid()) return origin(passenger);

        auto const& type = this->getTypeName();
        auto const  meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
        if (meta) {
            auto const& tab = meta->getPermissionTableConst();
            if (type == "minecraft:minecart" || type == "minecraft:boat") {
                if (tab.allowRideTrans) return origin(passenger);
            } else {
                if (tab.allowRideEntity) return origin(passenger);
            }
        }

        if (passenger.isPlayer()) {
            auto pl = passenger.getWeakEntity().tryUnwrap<Player>();
            if (pl.has_value()) {
                if (CheckPerm(nullptr, pps.getPlotID(), pl->getUuid().asString())) return origin(passenger);
            }
        }

        return false;
    }
    CATCH_RET(false);
}


// 凋零破坏方块
LL_TYPE_INSTANCE_HOOK(
    WitherDestroyHook,
    HookPriority::Normal,
    WitherBoss,
    &WitherBoss::_destroyBlocks,
    void,
    Level&                       level,
    AABB const&                  bb,
    BlockSource&                 region,
    int                          range,
    WitherBoss::WitherAttackType type
) {
    try {
        if (region.getDimensionId() != getPlotDimensionId()) return origin(level, bb, region, range, type);

        debugger("[凋零破坏方块] executed!");

        Cube  c(bb.min, bb.max);
        auto  land = c.getRangedPlots();
        auto& db   = PlotDBStorage::getInstance();

        for (auto const& p : land) {
            if (p.isCubeOnBorder(c)) {
                my_plugin::MyPlugin::getInstance().getSelf().getLogger().warn(
                    "Wither try to destroy block on border of plot {} at {}",
                    p.getPlotID(),
                    bb.toString()
                );
                return; // 禁止破坏边框
            }

            auto meta = db.getPlot(p.getPlotID());
            if (meta) {
                if (!meta->getPermissionTableConst().allowWitherDestroy) {
                    return;
                }
            } else return; // 如果地块不存在，则不进行破坏
        }

        return origin(level, bb, region, range, type);
    }
    CATCH;
}


// 活塞推动方块
LL_TYPE_INSTANCE_HOOK(
    PistonPushHook,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_attachedBlockWalker,
    bool,
    BlockSource&    region,
    BlockPos const& curPos, // 被推的方块位置
    uchar           curBranchFacing,
    uchar           pistonMoveFacing
) {
    try {
        if (region.getDimensionId() != getPlotDimensionId())
            return origin(region, curPos, curBranchFacing, pistonMoveFacing);

        debugger("[活塞推动方块] 目标: " << curPos.toString());

        auto sou = PPos(this->getPosition());
        auto tar = PPos(curPos);

        if (sou.isValid() && tar.isValid() && sou == tar) {
            if (!sou.isPosOnBorder(curPos) && !tar.isPosOnBorder(curPos)) {
                auto const meta = PlotDBStorage::getInstance().getPlot(sou.getPlotID());
                if (meta) {
                    if (meta->getPermissionTableConst().allowPistonPush)
                        return origin(region, curPos, curBranchFacing, pistonMoveFacing);
                }
            }
        }

        return false;
    }
    CATCH_RET(false);
}


// 红石更新
const auto RedStoneUpdateCallback =
    [](BlockSource& bs, BlockPos const& pos, int /* level */, bool /* isActive */) -> bool {
    if (bs.getDimensionId() != getPlotDimensionId()) return true;

    debugger("[RedstoneUpdate] pos: " << pos.toString());

    auto pps = PPos(pos);
    if (!pps.isValid()) return true;

    auto meta = PlotDBStorage::getInstance().getPlot(pps.getPlotID());
    if (meta) {
        if (meta->getPermissionTableConst().allowRedstoneUpdate) return true;
    }

    return false;
};
#define RedstoneUpdateHookMacro(NAME, TYPE, SYMBOL)                                                                    \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        NAME,                                                                                                          \
        HookPriority::Normal,                                                                                          \
        TYPE,                                                                                                          \
        SYMBOL,                                                                                                        \
        void,                                                                                                          \
        BlockSource&    region,                                                                                        \
        BlockPos const& pos,                                                                                           \
        int             strength,                                                                                      \
        bool            isFirstTime                                                                                    \
    ) {                                                                                                                \
        try {                                                                                                          \
            if (RedStoneUpdateCallback(region, pos, strength, isFirstTime)) {                                          \
                origin(region, pos, strength, isFirstTime);                                                            \
            }                                                                                                          \
        }                                                                                                              \
        CATCH;                                                                                                         \
    }

RedstoneUpdateHookMacro(
    RedstoneUpdateHook1,
    RedStoneWireBlock,
    "?onRedstoneUpdate@RedStoneWireBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);
RedstoneUpdateHookMacro(
    RedstoneUpdateHook2,
    DiodeBlock,
    "?onRedstoneUpdate@DiodeBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);
RedstoneUpdateHookMacro(
    RedstoneUpdateHook3,
    RedstoneTorchBlock,
    "?onRedstoneUpdate@RedstoneTorchBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);
RedstoneUpdateHookMacro(
    RedstoneUpdateHook4,
    ComparatorBlock,
    "?onRedstoneUpdate@ComparatorBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);


// 方块/实体爆炸
const auto ExplodeCallback = [](Actor* /* source */,
                                BlockSource& bs,
                                Vec3 const&  pos,
                                float        explosionRadius,
                                bool /* fire */,
                                bool /* breaksBlocks */,
                                float /* maxResistance */) -> bool {
    if (bs.getDimensionId() != getPlotDimensionId()) return true;

    debugger("[Explode] pos: " << pos.toString());

    Radius r(pos, explosionRadius + 1);
    auto   land = r.getRangedPlots();
    auto&  db   = PlotDBStorage::getInstance();

    for (auto& p : land) {
        if (p.isRadiusOnBorder(r)) {
            return false; // 禁止破坏边框
        }

        auto meta = db.getPlot(p.getPlotID());
        if (meta) {
            if (!meta->getPermissionTableConst().allowExplode) {
                return false; // 地皮禁止爆炸
            }
        }
    }

    return false; // 地皮世界禁止爆炸
};
LL_TYPE_INSTANCE_HOOK(
    ExplodeHook,
    HookPriority::Normal,
    Level,
    &Level::explode,
    bool,
    BlockSource& region,
    Actor*       source,
    Vec3 const&  pos,
    float        explosionRadius,
    bool         fire,
    bool         breaksBlocks,
    float        maxResistance,
    bool         allowUnderwater
) {
    try {
        if (ExplodeCallback(source, region, pos, explosionRadius, fire, breaksBlocks, maxResistance)) {
            return origin(region, source, pos, explosionRadius, fire, breaksBlocks, maxResistance, allowUnderwater);
        }
        return false;
    }
    CATCH_RET(false);
}


// Hook Manager
auto GetHooks() {
    return ll::memory::HookRegistrar<
        PressurePlateTriggerHook, // 生物踩踏压力板
        ActorRideHook,            // 生物骑乘
        WitherDestroyHook,        // 凋零破坏方块
        PistonPushHook,           // 活塞推动方块
        ExplodeHook,              // 方块/实体 爆炸
        MobHurtEffectHook,        // 生物受伤
        FarmDecayHook,            // 耕地退化
        // 玩家操作物品展示框
        PlayerUseFrameHook1,
        PlayerUseFrameHook2,
        // 弹射物生成
        ProjectileSpawnHook1,
        ProjectileSpawnHook2,
        ProjectileSpawnHook3,
        // 红石更新
        RedstoneUpdateHook1,
        RedstoneUpdateHook2,
        RedstoneUpdateHook3,
        RedstoneUpdateHook4>();
}
void registerHook() { GetHooks().hook(); }
void unregisterHook() { GetHooks().unhook(); }


} // namespace plo::event
