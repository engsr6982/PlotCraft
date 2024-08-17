#pragma once
#include "Version.h"
#include "ll/api/Config.h"
#include "plotcraft/Macro.h"
#include "plotcraft/utils/EconomySystem.h"
#include <string>
#include <vector>


using string = std::string;

namespace plo::config {

enum class PlotGeneratorType : int { Default, Custom };
struct _Config {
    int version = CONFIG_VERSION;

    struct _Generator {
        PlotGeneratorType type = PlotGeneratorType::Default; // 生成器类型

        // Default Generator
        int    plotWidth   = 64; // 地皮大小
        int    roadWidth   = 5;  // 道路宽度
        int    subChunkNum = 4;  // 子区块数量（负责高度,n*16）
        string roadBlock   = "minecraft:cherry_planks";
        string fillBlock   = "minecraft:grass_block";
        string borderBlock = "minecraft:stone_block_slab";

        // Custom Generator
        int    cuPlotChunkNum     = 3; // >= 2
        string cuPlotTemplatePath = "";
    } generator;

    utils::EconomyConfig economy; // 经济系统配置

    struct PlotWorld {
        int    maxBuyPlotCount   = 25;   // 最大购买地皮数量
        int    buyPlotPrice      = 1000; // 购买地皮价格
        bool   inPlotCanFly      = true; // 地皮内可飞行
        double playerSellPlotTax = 0.1;  // 玩家出售地皮扣除税率

        bool spawnMob = false; // 地皮世界是否生成生物

        int    maxMergePlotCount    = 4;    // 最大合并地皮数量
        int    baseMergePlotPrice   = 1000; // 基础合并地皮价格
        double mergePriceMultiplier = 1.1;  // 合并价格倍率，默认为1.0（保持基础价格不变）

        struct EventListener {
            bool onSculkSpreadListener{true};
            bool onSculkBlockGrowthListener{true};

            std::vector<string> onUseItemOnWhiteList = {"minecraft:clock"};
        } eventListener;
    } plotWorld;

    struct SwitchDim {
        std::vector<float> overWorld = {0, 100, 0}; // 切换维度时传送的坐标

        std::vector<float> plotWorld = {0.5, 0, 0.5};
    } switchDim;

    std::vector<int> allowedPlotTeleportDim = {0, 1, 2, 3}; // 允许传送到地皮维度的维度列表
};

PLAPI extern _Config cfg;

PLAPI void loadConfig();

PLAPI void updateConfig();

PLAPI double calculateMergePlotPrice(int mergeCount);

} // namespace plo::config