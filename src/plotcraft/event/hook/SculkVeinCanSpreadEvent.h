#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"


namespace plo::event::hook {


class SculkVeinCanSpreadEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    Block const&    mBlock;
    BlockPos const& mPos;
    uchar           mFace;

public:
    constexpr explicit SculkVeinCanSpreadEvent(Block const& block, BlockPos const& pos, uchar face)
    : Cancellable(),
      mBlock(block),
      mPos(pos),
      mFace(face) {}


    Block const&    getBlock() const;
    BlockPos const& getPos() const;
    uchar           getFace() const;
};


} // namespace plo::event::hook