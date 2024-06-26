#include "CoreUtils.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "mc/world/level/levelgen/WorldGenerator.h"
#include <memory>


namespace plo::core_utils {


int getPlotDimensionId() {
#if defined(OVERWORLD)
    return 0;
#else
    return VanillaDimensions::fromString("plot");
#endif
}


} // namespace plo::core_utils