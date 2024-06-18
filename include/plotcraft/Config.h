#pragma once
#include "ll/api/Config.h"
#include "plotcraft/Macro.h"
#include "plotcraft/utils/Moneys.h"
#include <string>


using string = std::string;

namespace plo::config {

struct _Config {
    int version = 1;

    struct _Generator {
        int plotWidth  = 64;  // 地皮大小
        int roadWidth  = 5;   // 道路宽度
        int generatorY = -61; // 生成层

        string roadBlock   = "minecraft:cherry_planks";
        string fillBlock   = "minecraft:grass_block";
        string borderBlock = "minecraft:stone_block_slab";
    } generator;
    utils::MoneysConfig moneys; // 经济系统配置
    struct _Func {
        int  maxBuyPlotCount = 10;    // 最大购买地皮数量
        int  buyPlotPrice    = 10000; // 购买地皮价格
        bool inPlotCanFly    = true;  // 地皮内可飞行
    } func;
};

PLAPI extern _Config cfg;

PLAPI void loadConfig();

} // namespace plo::config