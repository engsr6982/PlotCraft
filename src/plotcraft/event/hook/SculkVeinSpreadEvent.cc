#include "SculkVeinSpreadEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/IBlockWorldGenAPI.h"
#include "mc/world/level/block/utils/SculkVeinBlockBehavior.h"


namespace plo::event::hook {


BlockPos const&           SculkVeinSpreadEvent::getPos() const { return mPos; }
optional_ref<BlockSource> SculkVeinSpreadEvent::getBlockSource() const { return mBlockSource; }


LL_STATIC_HOOK(
    SculkVeinSpreadEventHook,
    ll::memory::HookPriority::Normal,
    "?_attemptPlaceSculk@SculkVeinBlockBehavior@@CA_NAEAVIBlockWorldGenAPI@@PEAVBlockSource@@AEBVBlockPos@@"
    "AEAVSculkSpreader@@AEAVRandom@@@Z",
    bool,
    IBlockWorldGenAPI& target,
    BlockSource*       region,
    BlockPos const&    pos,
    SculkSpreader&     idk,
    Random&            random
) {
    auto ev = SculkVeinSpreadEvent(pos, region);
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) {
        return false;
    }

    return origin(target, region, pos, idk, random);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&);
class SculkVeinSpreadEventEmitter : public ll::event::Emitter<emitterFactory, SculkVeinSpreadEvent> {
    ll::memory::HookRegistrar<SculkVeinSpreadEventHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&) {
    return std::make_unique<SculkVeinSpreadEventEmitter>();
}


} // namespace plo::event::hook