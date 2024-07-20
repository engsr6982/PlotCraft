#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"


namespace plo::event::hook {


class SculkBlockGrowthEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    BlockSource*    mSource;
    BlockPos const& mPos;

public:
    constexpr explicit SculkBlockGrowthEvent(BlockSource* source, BlockPos const& pos)
    : Cancellable(),
      mSource(source),
      mPos(pos) {}

    BlockPos const& getPos() const;
    BlockSource*    getSource() const;
};


} // namespace plo::event::hook