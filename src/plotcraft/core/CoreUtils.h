#include "mc/world/level/levelgen/WorldGenerator.h"
#include <iostream>


#ifdef GEN_1
#include "PlotGenerator.h"
#endif

#ifdef GEN_2
#include "PlotGenerator2.h"
#endif


namespace plo::core_utils {


using Generator = core::PlotGenerator;


int getPlotDimensionId();


} // namespace plo::core_utils
