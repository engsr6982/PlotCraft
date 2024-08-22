#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"


namespace more_events {


class SculkBlockGrowthBeforeEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    optional_ref<BlockSource> mBlockSource;
    BlockPos const&           mPos;

public:
    constexpr explicit SculkBlockGrowthBeforeEvent(optional_ref<BlockSource> source, BlockPos const& pos)
    : Cancellable(),
      mBlockSource(source),
      mPos(pos) {}

    BlockPos const&           getPos() const;
    optional_ref<BlockSource> getBlockSource() const;
};


class SculkBlockGrowthAfterEvent final : public ll::event::Event {
protected:
    optional_ref<BlockSource> mBlockSource;
    BlockPos const&           mPos;

public:
    constexpr explicit SculkBlockGrowthAfterEvent(optional_ref<BlockSource> source, BlockPos const& pos)
    : mBlockSource(source),
      mPos(pos) {}

    BlockPos const&           getPos() const;
    optional_ref<BlockSource> getBlockSource() const;
};


} // namespace more_events