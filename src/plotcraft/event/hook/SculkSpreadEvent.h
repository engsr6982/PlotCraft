#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"


namespace plo::event::hook {


class SculkSpreadEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    BlockPos const&           mPos;
    optional_ref<BlockSource> mBlockSource;

public:
    constexpr explicit SculkSpreadEvent(BlockPos const& pos, optional_ref<BlockSource> blockSource)
    : Cancellable(),
      mPos(pos),
      mBlockSource(blockSource) {}

    BlockPos const&           getPos() const;
    optional_ref<BlockSource> getBlockSource() const;
};


} // namespace plo::event::hook