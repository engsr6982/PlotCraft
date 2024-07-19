#include "SculkVeinCanSpreadEvent.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EmitterBase.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/IBlockWorldGenAPI.h"
#include "mc/world/level/block/utils/SculkVeinMultifaceSpreader.h"


namespace plo::event::hook {


Block const&    SculkVeinCanSpreadEvent::getBlock() const { return mBlock; }
BlockPos const& SculkVeinCanSpreadEvent::getPos() const { return mPos; }
uchar           SculkVeinCanSpreadEvent::getFace() const { return mFace; }


LL_TYPE_INSTANCE_HOOK(
    SculkVeinCanSpreadEventHook,
    ll::memory::HookPriority::Normal,
    SculkVeinMultifaceSpreader,
    "?_canSpreadInto@SculkVeinMultifaceSpreader@@UEBA_NAEAVIBlockWorldGenAPI@@AEBVBlock@@AEBVBlockPos@@E@Z",
    bool,
    IBlockWorldGenAPI& a1, // target
    Block const&       a2, // self   要蔓延的方块
    BlockPos const&    a3, // pos    要蔓延的方块的位置
    uchar              a4  // face   要蔓延的面
) {

    auto ev = SculkVeinCanSpreadEvent(a2, a3, a4);
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) {
        return false;
    }

    return origin(a1, a2, a3, a4);
}


static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&);
class SculkVeinCanSpreadEventEmitter : public ll::event::Emitter<emitterFactory, SculkVeinCanSpreadEvent> {
    ll::memory::HookRegistrar<SculkVeinCanSpreadEventHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&) {
    return std::make_unique<SculkVeinCanSpreadEventEmitter>();
}


} // namespace plo::event::hook