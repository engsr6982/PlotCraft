#pragma once
#include "ll/api/event/Event.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"
#include "plotcraft/PlotPos.h"


namespace plo::event {


class PlayerEnterPlot final : public ll::event::Event {
private:
    PlotPos mPos;    // 地皮坐标
    Player* mPlayer; // 玩家指针

public:
    PLAPI PlayerEnterPlot(const PlotPos& pos, Player* player);

    PLAPI Player* getPlayer() const;
    PLAPI PlotPos getPos() const;
};


class PlayerLeavePlot final : public ll::event::Event {
private:
    PlotPos mPos;    // 地皮坐标
    Player* mPlayer; // 玩家指针

public:
    PLAPI PlayerLeavePlot(const PlotPos& pos, Player* player);

    PLAPI Player* getPlayer() const;
    PLAPI PlotPos getPos() const;
};


} // namespace plo::event