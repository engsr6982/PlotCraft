#include "mc/enums/GameType.h"
#include "mc/math/Vec3.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/DataBase.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include <utility>
#include <vector>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702 4172)
#endif
#include "RemoteCallAPI.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace plo::remote {


void exportPLAPI();

void exportPLEvent();


int getPlotWorldDimid();


} // namespace plo::remote