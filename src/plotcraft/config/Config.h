#pragma once
#include "ll/api/Config.h"
#include <string>

using string = std::string;

namespace plotcraft::config {

struct _Config {
    int version = 1;

    struct _Generator {
        int plotWidth  = 16;  // 地皮大小
        int roadWidth  = 3;   // 道路宽度
        int generatorY = -61; // 生成层

        string roadBlock   = "minecraft:cherry_planks";
        string fillBlock   = "minecraft:grass_block";
        string borderBlock = "minecraft:stone_block_slab";
    } generator;
};

extern _Config cfg;

void loadConfig();

} // namespace plotcraft::config