#include "plotcraft/event/PlayerPlotEvent.h"

namespace plo::event {

// PlayerEnterPlot
PlayerEnterPlot::PlayerEnterPlot(const PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}

Player* PlayerEnterPlot::getPlayer() const { return mPlayer; }

PlotPos PlayerEnterPlot::getPos() const { return mPos; }


// PlayerLeavePlot
PlayerLeavePlot::PlayerLeavePlot(const PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}

Player* PlayerLeavePlot::getPlayer() const { return mPlayer; }

PlotPos PlayerLeavePlot::getPos() const { return mPos; }


} // namespace plo::event