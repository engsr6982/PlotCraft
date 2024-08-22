#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"

namespace more_events {


class PlayerDropItemEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    Player&          mPlayer;
    ItemStack const& mItemStack;

public:
    constexpr explicit PlayerDropItemEvent(Player& player, ItemStack const& itemStack)
    : Cancellable(),
      mPlayer(player),
      mItemStack(itemStack) {}

    Player&          getPlayer() const;
    ItemStack const& getItemStack() const;
};


} // namespace more_events