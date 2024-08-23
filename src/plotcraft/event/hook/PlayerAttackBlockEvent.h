#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"


namespace more_events {


class PlayerAttackBlockEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    BlockPos const&      mPos;
    optional_ref<Player> mPlayer;

public:
    constexpr explicit PlayerAttackBlockEvent(BlockPos const& pos, optional_ref<Player> player)
    : Cancellable(),
      mPos(pos),
      mPlayer(player) {}

    BlockPos const&      getPos() const;
    optional_ref<Player> getPlayer() const;
};


} // namespace more_events