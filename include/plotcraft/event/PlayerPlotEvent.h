#pragma once
#include "ll/api/event/Event.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/PlotPos.h"


namespace plo::event {


class PlayerEnterPlot final : public ll::event::Event {
private:
    plo::core::PlotPos mPos;    // 地皮坐标
    Player*            mPlayer; // 玩家指针

public:
    PlayerEnterPlot(const plo::core::PlotPos& pos, Player* player);

    Player* getPlayer() const;

    plo::core::PlotPos getPos() const;
};


class PlayerLeavePlot final : public ll::event::Event {
private:
    plo::core::PlotPos mPos;    // 地皮坐标
    Player*            mPlayer; // 玩家指针

public:
    PlayerLeavePlot(const plo::core::PlotPos& pos, Player* player);

    Player* getPlayer() const;

    plo::core::PlotPos getPos() const;
};


} // namespace plo::event