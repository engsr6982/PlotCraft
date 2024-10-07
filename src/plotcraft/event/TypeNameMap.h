#pragma once
#include "plotcraft/Global.h"


namespace plot::event::TypeNameMap {


inline static std::unordered_set<string> UseItemOnMap = {
    "minecraft:bed",                        // 床
    "minecraft:chest",                      // 箱子
    "minecraft:trapped_chest",              // 陷阱箱
    "minecraft:crafting_table",             // 制作台
    "minecraft:campfire",                   // 营火
    "minecraft:soul_campfire",              // 灵魂营火
    "minecraft:composter",                  // 垃圾箱
    "minecraft:noteblock",                  // 音符盒
    "minecraft:jukebox",                    // 唱片机
    "minecraft:bell",                       // 钟
    "minecraft:daylight_detector",          // 阳光探测器
    "minecraft:daylight_detector_inverted", // 阳光探测器(夜晚)
    "minecraft:lectern",                    // 讲台
    "minecraft:cauldron",                   // 炼药锅
    "minecraft:lever",                      // 拉杆
    "minecraft:dragon_egg",                 // 龙蛋
    "minecraft:flower_pot",                 // 花盆
    "minecraft:respawn_anchor",             // 重生锚
    "minecraft:glow_ink_sac",               // 荧光墨囊
    "minecraft:end_crystal",                // 末地水晶
    "minecraft:ender_eye",                  // 末影之眼
    "minecraft:flint_and_steel",            // 打火石
    "minecraft:skull",                      // 头颅
    "minecraft:banner",                     // 旗帜
    "minecraft:bone_meal",                  // 骨粉
    "minecraft:minecart",                   // 矿车
    "minecraft:armor_stand",                // 盔甲架

    "minecraft:axolotl_bucket",       // 美西螈桶
    "minecraft:powder_snow_bucket",   // 细雪桶
    "minecraft:pufferfish_bucket",    // 河豚桶
    "minecraft:tropical_fish_bucket", // 热带鱼桶
    "minecraft:salmon_bucket",        // 桶装鲑鱼
    "minecraft:cod_bucket",           // 鳕鱼桶
    "minecraft:water_bucket",         // 水桶
    "minecraft:cod_bucket",           // 鳕鱼桶
    "minecraft:lava_bucket",          // 熔岩桶
    "minecraft:bucket",               // 桶

    "minecraft:shulker_box",            // 潜影盒
    "minecraft:undyed_shulker_box",     // 未染色的潜影盒
    "minecraft:white_shulker_box",      // 白色潜影盒
    "minecraft:orange_shulker_box",     // 橙色潜影盒
    "minecraft:magenta_shulker_box",    // 品红色潜影盒
    "minecraft:light_blue_shulker_box", // 浅蓝色潜影盒
    "minecraft:yellow_shulker_box",     // 黄色潜影盒
    "minecraft:lime_shulker_box",       // 黄绿色潜影盒
    "minecraft:pink_shulker_box",       // 粉红色潜影盒
    "minecraft:gray_shulker_box",       // 灰色潜影盒
    "minecraft:light_gray_shulker_box", // 浅灰色潜影盒
    "minecraft:cyan_shulker_box",       // 青色潜影盒
    "minecraft:purple_shulker_box",     // 紫色潜影盒
    "minecraft:blue_shulker_box",       // 蓝色潜影盒
    "minecraft:brown_shulker_box",      // 棕色潜影盒
    "minecraft:green_shulker_box",      // 绿色潜影盒
    "minecraft:red_shulker_box",        // 红色潜影盒
    "minecraft:black_shulker_box",      // 黑色潜影盒

    "minecraft:stone_button",               // 石头按钮
    "minecraft:wooden_button",              // 木头按钮
    "minecraft:spruce_button",              // 云杉木按钮
    "minecraft:birch_button",               // 白桦木按钮
    "minecraft:jungle_button",              // 丛林木按钮
    "minecraft:acacia_button",              // 金合欢木按钮
    "minecraft:dark_oak_button",            // 深色橡木按钮
    "minecraft:crimson_button",             // 绯红木按钮
    "minecraft:warped_button",              // 诡异木按钮
    "minecraft:polished_blackstone_button", // 磨制黑石按钮
    "minecraft:mangrove_button",            // 红树木按钮
    "minecraft:cherry_button",              // 樱花木按钮
    "minecraft:bamboo_button",              // 竹按钮

    "minecraft:trapdoor",                        // 活板门
    "minecraft:spruce_trapdoor",                 // 云杉木活板门
    "minecraft:birch_trapdoor",                  // 白桦木活板门
    "minecraft:jungle_trapdoor",                 // 丛林木活板门
    "minecraft:acacia_trapdoor",                 // 金合欢木活板门
    "minecraft:dark_oak_trapdoor",               // 深色橡木活板门
    "minecraft:crimson_trapdoor",                // 绯红木活板门
    "minecraft:warped_trapdoor",                 // 诡异木活板门
    "minecraft:copper_trapdoor",                 // 铜活板门
    "minecraft:exposed_copper_trapdoor",         // 斑驳的铜活板门
    "minecraft:weathered_copper_trapdoor",       // 锈蚀的铜活板门
    "minecraft:oxidized_copper_trapdoor",        // 氧化的铜活板门
    "minecraft:waxed_copper_trapdoor",           // 涂蜡的铜活板门
    "minecraft:waxed_exposed_copper_trapdoor",   // 涂蜡的斑驳的铜活板门
    "minecraft:waxed_weathered_copper_trapdoor", // 涂蜡的锈蚀的铜活板门
    "minecraft:waxed_oxidized_copper_trapdoor",  // 涂蜡的氧化的铜活板门
    "minecraft:mangrove_trapdoor",               // 红树木活板门
    "minecraft:cherry_trapdoor",                 // 樱树木活板门
    "minecraft:bamboo_trapdoor",                 // 竹活板门

    "minecraft:fence_gate",          // 栅栏门
    "minecraft:spruce_fence_gate",   // 云杉木栅栏门
    "minecraft:birch_fence_gate",    // 白桦木栅栏门
    "minecraft:jungle_fence_gate",   // 丛林木栅栏门
    "minecraft:acacia_fence_gate",   // 金合欢木栅栏门
    "minecraft:dark_oak_fence_gate", // 深色橡木栅栏门
    "minecraft:crimson_fence_gate",  // 绯红木栅栏门
    "minecraft:warped_fence_gate",   // 诡异木栅栏门
    "minecraft:mangrove_fence_gate", // 红树木栅栏门
    "minecraft:cherry_fence_gate",   // 樱树木栅栏门
    "minecraft:bamboo_fence_gate",   // 竹栅栏门

    "minecraft:wooden_door",   // 橡木门
    "minecraft:spruce_door",   // 云杉木门
    "minecraft:birch_door",    // 白桦木门
    "minecraft:jungle_door",   // 丛林木门
    "minecraft:acacia_door",   // 金合欢木门
    "minecraft:dark_oak_door", // 深色橡木门
    "minecraft:crimson_door",  // 绯红木门
    "minecraft:warped_door",   // 诡异木门
    "minecraft:mangrove_door", // 红树木门
    "minecraft:cherry_door",   // 樱树木门
    "minecraft:bamboo_door",   // 竹门

    "minecraft:wooden_axe",       // 木斧
    "minecraft:stone_axe",        // 石斧
    "minecraft:iron_axe",         // 铁斧
    "minecraft:golden_axe",       // 金斧
    "minecraft:diamond_axe",      // 钻石斧
    "minecraft:netherite_axe",    // 下界合金斧
    "minecraft:wooden_hoe",       // 木锄
    "minecraft:stone_hoe",        // 石锄
    "minecraft:iron_hoe",         // 铁锄
    "minecraft:diamond_hoe",      // 钻石锄
    "minecraft:golden_hoe",       // 金锄
    "minecraft:netherite_hoe",    // 下界合金锄
    "minecraft:wooden_shovel",    // 木铲
    "minecraft:stone_shovel",     // 石铲
    "minecraft:iron_shovel",      // 铁铲
    "minecraft:diamond_shovel",   // 钻石铲
    "minecraft:golden_shovel",    // 金铲
    "minecraft:netherite_shovel", // 下界合金铲

    "minecraft:standing_sign",          // 站立的告示牌
    "minecraft:spruce_standing_sign",   // 站立的云杉木告示牌
    "minecraft:birch_standing_sign",    // 站立的白桦木告示牌
    "minecraft:jungle_standing_sign",   // 站立的丛林木告示牌
    "minecraft:acacia_standing_sign",   // 站立的金合欢木告示牌
    "minecraft:darkoak_standing_sign",  // 站立的深色橡木告示牌
    "minecraft:mangrove_standing_sign", // 站立的红树木告示牌
    "minecraft:cherry_standing_sign",   // 站立的樱树木告示牌
    "minecraft:bamboo_standing_sign",   // 站立的竹子告示牌
    "minecraft:crimson_standing_sign",  // 站立的绯红木告示牌
    "minecraft:warped_standing_sign",   // 站立的诡异木告示牌
    "minecraft:wall_sign",              // 墙上的告示牌
    "minecraft:spruce_wall_sign",       // 墙上的云杉木告示牌
    "minecraft:birch_wall_sign",        // 墙上的白桦木告示牌
    "minecraft:jungle_wall_sign",       // 墙上的丛林木告示牌
    "minecraft:acacia_wall_sign",       // 墙上的金合欢木告示牌
    "minecraft:darkoak_wall_sign",      // 墙上的深色橡木告示牌
    "minecraft:mangrove_wall_sign",     // 墙上的红树木告示牌
    "minecraft:cherry_wall_sign",       // 墙上的樱树木告示牌
    "minecraft:bamboo_wall_sign",       // 墙上的竹子告示牌
    "minecraft:crimson_wall_sign",      // 墙上的绯红木告示牌
    "minecraft:warped_wall_sign"        // 墙上的诡异木告示牌
};
inline static std::unordered_set<string> InteractBlockMap = {
    "minecraft:cartography_table", // 制图台
    "minecraft:smithing_table",    // 锻造台
    "minecraft:furnace",           // 熔炉
    "minecraft:blast_furnace",     // 高炉
    "minecraft:smoker",            // 烟熏炉
    "minecraft:brewing_stand",     // 酿造台
    "minecraft:anvil",             // 铁砧
    "minecraft:grindstone",        // 砂轮
    "minecraft:enchanting_table",  // 附魔台
    "minecraft:barrel",            // 木桶
    "minecraft:beacon",            // 信标
    "minecraft:hopper",            // 漏斗
    "minecraft:dropper",           // 投掷器
    "minecraft:dispenser",         // 发射器
    "minecraft:loom",              // 织布机
    "minecraft:stonecutter_block", // 切石机
    "minecraft:lit_furnace",       // 燃烧中的熔炉
    "minecraft:lit_blast_furnace", // 燃烧中的高炉
    "minecraft:lit_smoker"         // 燃烧中的烟熏炉
};
inline static std::unordered_set<string> AnimalEntityMap = {
    "minecraft:axolotl",          // 美西螈
    "minecraft:bat",              // 蝙蝠
    "minecraft:cat",              // 猫
    "minecraft:chicken",          // 鸡
    "minecraft:cod",              // 鳕鱼
    "minecraft:cow",              // 牛
    "minecraft:donkey",           // 驴
    "minecraft:fox",              // 狐狸
    "minecraft:glow_squid",       // 发光鱿鱼
    "minecraft:horse",            // 马
    "minecraft:mooshroom",        // 蘑菇牛
    "minecraft:mule",             // 驴
    "minecraft:ocelot",           // 豹猫
    "minecraft:parrot",           // 鹦鹉
    "minecraft:pig",              // 猪
    "minecraft:rabbit",           // 兔子
    "minecraft:salmon",           // 鲑鱼
    "minecraft:snow_golem",       // 雪傀儡
    "minecraft:sheep",            // 羊
    "minecraft:skeleton_horse",   // 骷髅马
    "minecraft:squid",            // 鱿鱼
    "minecraft:strider",          // 炽足兽
    "minecraft:tropical_fish",    // 热带鱼
    "minecraft:turtle",           // 海龟
    "minecraft:villager_v2",      // 村民
    "minecraft:wandering_trader", // 流浪商人
    "minecraft:npc"               // NPC
};


} // namespace plot::event::TypeNameMap