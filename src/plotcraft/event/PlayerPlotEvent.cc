#include "PlayerPlotEvent.h"

namespace plo::event {

// PlayerEnterPlot
PlayerEnterPlot::PlayerEnterPlot(const plo::core::PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}

Player* PlayerEnterPlot::getPlayer() const { return mPlayer; }

plo::core::PlotPos PlayerEnterPlot::getPos() const { return mPos; }


// PlayerLeavePlot
PlayerLeavePlot::PlayerLeavePlot(const plo::core::PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}

Player* PlayerLeavePlot::getPlayer() const { return mPlayer; }

plo::core::PlotPos PlayerLeavePlot::getPos() const { return mPos; }


} // namespace plo::event