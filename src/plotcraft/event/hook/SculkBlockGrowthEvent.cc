#include "SculkBlockGrowthEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/IBlockWorldGenAPI.h"
#include "mc/world/level/block/utils/SculkBlockBehavior.h"


namespace plo::event::hook {


BlockSource*    SculkBlockGrowthEvent::getSource() const { return mSource; }
BlockPos const& SculkBlockGrowthEvent::getPos() const { return mPos; }


LL_TYPE_STATIC_HOOK(
    SculkBlockGrowthHook,
    ll::memory::HookPriority::Normal,
    SculkBlockBehavior,
    // "?_placeGrowthAt@SculkBlockBehavior@@CAXAEAVIBlockWorldGenAPI@@PEAVBlockSource@@AEBVBlockPos@@AEAVRandom@@"
    // "AEAVSculkSpreader@@@Z",
    &SculkBlockBehavior::_placeGrowthAt,
    void,
    IBlockWorldGenAPI& a1, // target
    BlockSource*       a2, // region
    BlockPos const&    a3, // pos
    Random&            a4, // random
    SculkSpreader&     a5
) {
    auto ev = SculkBlockGrowthEvent(a2, a3);
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) {
        return;
    }

    origin(a1, a2, a3, a4, a5);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&);
class SculkBlockGrowthEventEmitter : public ll::event::Emitter<emitterFactory, SculkBlockGrowthEvent> {
    ll::memory::HookRegistrar<SculkBlockGrowthHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&) {
    return std::make_unique<SculkBlockGrowthEventEmitter>();
}


} // namespace plo::event::hook