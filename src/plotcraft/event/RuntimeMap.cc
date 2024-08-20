#include "RuntimeMap.h"
#include <vector>

namespace plo::event {


std::unordered_map<MapType, std::vector<std::string>> RuntimeMap::mData = {
    {MapType::UseItem,
     {
         "minecraft:bed",                        // 床
         "minecraft:chest",                      // 箱子
         "minecraft:trapped_chest",              // 陷阱箱
         "minecraft:crafting_table",             // 制作台
         "minecraft:campfire",                   // 营火
         "minecraft:soul_campfire",              // 灵魂营火
         "minecraft:composter",                  // 垃圾箱
         "minecraft:undyed_shulker_box",         // 未染色的潜影盒
         "minecraft:shulker_box",                // 潜影盒
         "minecraft:noteblock",                  // 音符盒
         "minecraft:jukebox",                    // 唱片机
         "minecraft:bell",                       // 钟
         "minecraft:daylight_detector_inverted", // 阳光探测器(夜晚)
         "minecraft:daylight_detector",          // 阳光探测器
         "minecraft:lectern",                    // 讲台
         "minecraft:cauldron",                   // 炼药锅
         "minecraft:lever",                      // 拉杆
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
         "minecraft:respawn_anchor",             // 重生锚
         "minecraft:trapdoor",                   // 活板门
         "minecraft:spruce_trapdoor",            // 云杉木活板门
         "minecraft:birch_trapdoor",             // 白桦木活板门
         "minecraft:jungle_trapdoor",            // 丛林木活板门
         "minecraft:acacia_trapdoor",            // 金合欢木活板门
         "minecraft:dark_oak_trapdoor",          // 深色橡木活板门
         "minecraft:crimson_trapdoor",           // 绯红木活板门
         "minecraft:warped_trapdoor",            // 诡异木活板门
         "minecraft:fence_gate",                 // 栅栏门
         "minecraft:spruce_fence_gate",          // 云杉木栅栏门
         "minecraft:birch_fence_gate",           // 白桦木栅栏门
         "minecraft:jungle_fence_gate",          // 丛林木栅栏门
         "minecraft:acacia_fence_gate",          // 金合欢木栅栏门
         "minecraft:dark_oak_fence_gate",        // 深色橡木栅栏门
         "minecraft:crimson_fence_gate",         // 绯红木栅栏门
         "minecraft:warped_fence_gate",          // 诡异木栅栏门
         "minecraft:wooden_door",                // 橡木门
         "minecraft:spruce_door",                // 云杉木门
         "minecraft:birch_door",                 // 白桦木门
         "minecraft:jungle_door",                // 丛林木门
         "minecraft:acacia_door",                // 金合欢木门
         "minecraft:dark_oak_door",              // 深色橡木门
         "minecraft:crimson_door",               // 绯红木门
         "minecraft:warped_door",                // 诡异木门
         "minecraft:dragon_egg",                 // 龙蛋
         "minecraft:flower_pot"                  // 花盆
     }                   },
    {MapType::BlockInteracted,
     {
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
     }                   },
    {MapType::ItemWhiteList,
     {"minecraft:glow_ink_sac",         // 荧光墨囊
      "minecraft:end_crystal",          // 末地水晶
      "minecraft:ender_eye",            // 末影之眼
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
      "minecraft:flint_and_steel",      // 打火石
      "minecraft:skull",                // 头颅
      "minecraft:wooden_axe",           // 木斧
      "minecraft:stone_axe",            // 石斧
      "minecraft:iron_axe",             // 铁斧
      "minecraft:golden_axe",           // 金斧
      "minecraft:diamond_axe",          // 钻石斧
      "minecraft:netherite_axe",        // 下界合金斧
      "minecraft:banner"}},
    {MapType::SpecialAttack,
     {
         "minecraft:ender_crystal", // 末地水晶
         "minecraft:armor_stand"    // 盔甲架
     }                   },
    {MapType::AnimalEntity,
     {
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
     }                   },
    {MapType::MobEntity,
     {
         // type A
         "minecraft:pufferfish",    // 河豚
         "minecraft:bee",           // 蜜蜂
         "minecraft:dolphin",       // 海豚
         "minecraft:goat",          // 山羊
         "minecraft:iron_golem",    // 铁傀儡
         "minecraft:llama",         // 羊驼
         "minecraft:llama_spit",    // 羊驼唾沫
         "minecraft:wolf",          // 狼
         "minecraft:panda",         // 熊猫
         "minecraft:polar_bear",    // 北极熊
         "minecraft:enderman",      // 末影人
         "minecraft:piglin",        // 猪灵
         "minecraft:spider",        // 蜘蛛
         "minecraft:cave_spider",   // 洞穴蜘蛛
         "minecraft:zombie_pigman", // 僵尸猪人

         // type B
         "minecraft:blaze",                 // 烈焰人
         "minecraft:small_fireball",        // 小火球
         "minecraft:creeper",               // 爬行者
         "minecraft:drowned",               // 溺尸
         "minecraft:elder_guardian",        // 远古守卫者
         "minecraft:endermite",             // 末影螨
         "minecraft:evocation_illager",     // 唤魔者
         "minecraft:evocation_fang",        // 唤魔者尖牙
         "minecraft:ghast",                 // 恶魂
         "minecraft:fireball",              // 火球
         "minecraft:guardian",              // 守卫者
         "minecraft:hoglin",                // 疣猪兽
         "minecraft:husk",                  // 凋零骷髅
         "minecraft:magma_cube",            // 岩浆怪
         "minecraft:phantom",               // 幻翼
         "minecraft:pillager",              // 掠夺者
         "minecraft:ravager",               // 劫掠兽
         "minecraft:shulker",               // 潜影贝
         "minecraft:shulker_bullet",        // 潜影贝弹射物
         "minecraft:silverfish",            // 蠹虫
         "minecraft:skeleton",              // 骷髅
         "minecraft:skeleton_horse",        // 骷髅马
         "minecraft:slime",                 // 史莱姆
         "minecraft:vex",                   // 恶魂兽
         "minecraft:vindicator",            // 卫道士
         "minecraft:witch",                 // 女巫
         "minecraft:wither_skeleton",       // 凋零骷髅
         "minecraft:zoglin",                // 疣猪兽
         "minecraft:zombie",                // 僵尸
         "minecraft:zombie_villager_v2",    // 僵尸村民
         "minecraft:piglin_brute",          // 猪灵蛮兵
         "minecraft:ender_dragon",          // 末影龙
         "minecraft:dragon_fireball",       // 末影龙火球
         "minecraft:wither",                // 凋零
         "minecraft:wither_skull",          // 凋零之首
         "minecraft:wither_skull_dangerous" // 蓝色凋灵之首(Wiki)
     }                   },
    {MapType::AttackBlock,
     {
         "minecraft:dragon_egg" // 龙蛋
     }                   }
};

bool RuntimeMap::has(MapType type, std::string const& typeName) {
    auto& v = mData[type];
    return std::find(v.begin(), v.end(), typeName) != v.end();
}


} // namespace plo::event