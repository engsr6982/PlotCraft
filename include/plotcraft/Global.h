#pragma once
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/level/BlockPos.h"
#include <nlohmann/json.hpp>
#include <string>


#ifdef PLOT_EXPORTS
#define PLAPI __declspec(dllexport)
#else
#define PLAPI __declspec(dllimport)
#endif


// Config
#define CONFIG_VERSION 9

// PlotMetadata
#define METADATA_VERSION 7

// PlayerSetting
#define SETTING_VERSION 1


namespace plot {


using string    = std::string;
using json      = nlohmann::json;
using DiagonPos = std::pair<Vec3, Vec3>;
using Vertexs   = std::vector<Vec3>;

using PlotID  = string; // PlotPos::getPlotID()
using RoadID  = string; // PlotRoad::getRoadID()
using CrossID = string; // PlotCross::getCrossID()

using UUIDm     = mce::UUID;
using UUIDs     = string;
using CommentID = int;


int getPlotWorldDimensionId();


} // namespace plot