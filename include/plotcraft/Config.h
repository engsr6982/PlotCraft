#pragma once
#include "ll/api/Config.h"
#include "plotcraft/Macro.h"
#include "plotcraft/utils/Moneys.h"
#include <string>
#include <vector>


using string = std::string;

namespace plo::config {

#define mConfigVersion 5

struct _Config {
    int version = mConfigVersion;

#ifdef GEN_1
    struct _Generator {
        int plotWidth   = 64; // 地皮大小
        int roadWidth   = 5;  // 道路宽度
        int subChunkNum = 4;  // 子区块数量（负责高度,n*16）

        string roadBlock   = "minecraft:cherry_planks";
        string fillBlock   = "minecraft:grass_block";
        string borderBlock = "minecraft:stone_block_slab";
    } generator;
#endif

#ifdef GEN_2
    struct _Generator {
        int plotChunkSize = 3; // 地皮区块
        int subChunkNum   = 0; // TODO: 子区块数量（负责高度,n*16）

        string roadBlock   = "minecraft:cherry_planks";
        string borderBlock = "minecraft:stone_block_slab";
    } generator;
#endif

    utils::MoneysConfig moneys; // 经济系统配置

    struct SwitchDim {
        std::vector<float> overWorld = {0, 100, 0}; // 切换维度时传送的坐标

        std::vector<float> plotWorld = {0.5, 0, 0.5};
    } switchDim;

    struct PlotWorld {
        int   maxBuyPlotCount   = 25;   // 最大购买地皮数量
        int   buyPlotPrice      = 1000; // 购买地皮价格
        bool  inPlotCanFly      = true; // 地皮内可飞行
        float playerSellPlotTax = 0.1;  // 玩家出售地皮扣除税率

        bool spawnMob = false; // 地皮世界是否生成生物
    } plotWorld;

    std::vector<int> allowedPlotTeleportDim = {0, 1, 2, 3}; // 允许传送到地皮维度的维度列表
};

PLAPI extern _Config cfg;

PLAPI void loadConfig();

PLAPI void updateConfig();

} // namespace plo::config