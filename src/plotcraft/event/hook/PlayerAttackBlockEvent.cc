#include "PlayerAttackBlockEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include <mc/world/level/block/Block.h>


namespace more_events {

BlockPos const&      PlayerAttackBlockEvent::getPos() const { return mPos; }
optional_ref<Player> PlayerAttackBlockEvent::getPlayer() const { return mPlayer; }


LL_TYPE_INSTANCE_HOOK(
    PlayerAttackBlockEventHook,
    HookPriority::Normal,
    Block,
    &Block::attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    auto before = PlayerAttackBlockEvent(pos, player);
    ll::event::EventBus::getInstance().publish(before);

    if (before.isCancelled()) {
        return false;
    }

    return origin(player, pos);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory1(ll::event::ListenerBase&);
class PlayerAttackBlockEventEmitter : public ll::event::Emitter<emitterFactory1, PlayerAttackBlockEvent> {
    ll::memory::HookRegistrar<PlayerAttackBlockEventHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory1(ll::event::ListenerBase&) {
    return std::make_unique<PlayerAttackBlockEventEmitter>();
}


} // namespace more_events