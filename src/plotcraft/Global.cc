#include "plotcraft/Global.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

namespace plot {

int getPlotWorldDimensionId() {
#if defined(OVERWORLD)
    return 0;
#else
    static int const dimensionId = VanillaDimensions::fromString("plot");
    return dimensionId;
#endif
}

} // namespace plot