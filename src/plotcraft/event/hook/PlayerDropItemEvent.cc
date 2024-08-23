#include "PlayerDropItemEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include <mc/world/inventory/transaction/ComplexInventoryTransaction.h>


namespace more_events {


Player&          PlayerDropItemEvent::getPlayer() const { return mPlayer; };
ItemStack const& PlayerDropItemEvent::getItemStack() const { return mItemStack; };


LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook1,
    HookPriority::Normal,
    Player,
    "?drop@Player@@UEAA_NAEBVItemStack@@_N@Z",
    bool,
    ItemStack const& item,
    bool             randomly
) {
    auto ev = PlayerDropItemEvent(*this, item);
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return false;

    return origin(item, randomly);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook2,
    HookPriority::Normal,
    ComplexInventoryTransaction,
    "?handle@ComplexInventoryTransaction@@UEBA?AW4InventoryTransactionError@@AEAVPlayer@@_N@Z",
    InventoryTransactionError,
    Player& player,
    bool    isSenderAuthority
) {
    if (type == ComplexInventoryTransaction::Type::NormalTransaction) {
        InventorySource source(InventorySourceType::ContainerInventory, ContainerID::Inventory);
        auto&           actions = data.getActions(source);
        if (actions.size() == 1) {
            auto ev =
                PlayerDropItemEvent(player, const_cast<ItemStack&>(player.getInventory().getItem(actions[0].mSlot)));
            ll::event::EventBus::getInstance().publish(ev);
            if (ev.isCancelled()) return InventoryTransactionError::NoError;
        }
    }
    return origin(player, isSenderAuthority);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&);
class PlayerDropItemEventEmitter : public ll::event::Emitter<emitterFactory, PlayerDropItemEvent> {
    ll::memory::HookRegistrar<PlayerDropItemHook1, PlayerDropItemHook2> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&) {
    return std::make_unique<PlayerDropItemEventEmitter>();
}


} // namespace more_events
