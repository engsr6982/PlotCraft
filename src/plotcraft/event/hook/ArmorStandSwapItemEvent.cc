#include "ArmorStandSwapItemEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/level/block/Block.h>


namespace more_events {

Puv::Legacy::EquipmentSlot ArmorStandSwapItemEvent::getSlot() const { return mSlot; }
Player&                    ArmorStandSwapItemEvent::getPlayer() const { return mPlayer; }
Mob&                       ArmorStandSwapItemEvent::getArmorStand() const { return mArmorStand; };


LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                    player,
    Puv::Legacy::EquipmentSlot slot
) {
    auto before = ArmorStandSwapItemEvent(slot, player, *this);
    ll::event::EventBus::getInstance().publish(before);

    if (before.isCancelled()) {
        return false;
    }

    return origin(player, slot);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory1(ll::event::ListenerBase&);
class ArmorStandSwapItemEventEmitter : public ll::event::Emitter<emitterFactory1, ArmorStandSwapItemEvent> {
    ll::memory::HookRegistrar<ArmorStandSwapItemHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory1(ll::event::ListenerBase&) {
    return std::make_unique<ArmorStandSwapItemEventEmitter>();
}


} // namespace more_events