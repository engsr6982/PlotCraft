#pragma once

namespace plot {

enum class PlotDirection : int {
    Unknown = -1, // 未知
    North   = 0,  // 北     z-
    East    = 1,  // 东     x+
    South   = 2,  // 南     z+
    West    = 3,  // 西     x-
    NE      = 4,  // 东北   z- x+
    SE      = 5,  // 东南   z+ x+
    SW      = 6,  // 西南   z+ x-
    NW      = 7,  // 西北   z- x-
};

}