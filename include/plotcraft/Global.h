#pragma once
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/level/BlockPos.h"
#include <nlohmann/json.hpp>
#include <string>


namespace plo {


using string    = std::string;
using json      = nlohmann::json;
using DiagonPos = std::pair<Vec3, Vec3>;
using Vertexs   = std::vector<Vec3>;

using PlotID  = string; // PPos::getPlotID()
using RoadID  = string; // PlotRoad::getRoadID()
using CrossID = string; // PlotCross::getCrossID()

using UUIDm     = mce::UUID;
using UUIDs     = string;
using CommentID = int;


} // namespace plo