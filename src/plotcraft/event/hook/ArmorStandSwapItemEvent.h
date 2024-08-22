#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"


namespace more_events {


class ArmorStandSwapItemEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    Puv::Legacy::EquipmentSlot mSlot;
    Player&                    mPlayer;
    Mob&                       mArmorStand;

public:
    constexpr explicit ArmorStandSwapItemEvent(Puv::Legacy::EquipmentSlot slot, Player& player, Mob& armorStand)
    : Cancellable(),
      mSlot(slot),
      mPlayer(player),
      mArmorStand(armorStand) {}

    Puv::Legacy::EquipmentSlot getSlot() const;
    Player&                    getPlayer() const;
    Mob&                       getArmorStand() const;
};


} // namespace more_events