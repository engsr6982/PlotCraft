#include "plotcraft/core/Utils.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

namespace plo::core {

int getPlotDimensionId() {
#if defined(OVERWORLD)
    return 0;
#else
    return VanillaDimensions::fromString("plot");
#endif
}

} // namespace plo::core