# PlotCraft

适用于 BDS 的地皮插件，依赖`MoreDimension`和`LeviLamina`

-   专属的地皮世界维度
-   完善的地皮坐标计算
-   可控的地皮、道路生成
-   地皮出售、购买、评论
-   等....

## 使用

-   依赖：
-   MoreDimension
-   LegacyMoney
-   LegacyRemoteCall

```bash
lip install github.com/engsr6982/PlotCraft
```

> `/plugins/PlotCraft/lse/PlotCraft-Fixer.js`
> 此文件用于修复插件本体未处理的事件，如果需要使用请将其放入`plugins/`目录下, 由 LSE 引擎加载。

## 配置文件

```json
{
    "version": 3, // 配置文件版本(请勿修改)
    "generator": {
        // 地皮生成器配置
        // 生成器不支持动态修改，请一次确定好生成器配置再进入地皮世界
        // 如果动态修改，则100%导致已生成的区块和未生成区块之间前街错误（地形错误）
        "plotWidth": 64, // 地皮大小（正方形）
        "roadWidth": 5, // 道路宽度
        "subChunkNum": 4, // 子区块数量(生成的子区块数量，4 * 16 = 64 即世界高度为 0 )
        "roadBlock": "minecraft:cherry_planks", // 道路方块
        "fillBlock": "minecraft:grass_block", // 填充方块
        "borderBlock": "minecraft:stone_block_slab" // 边界方块
    },
    "moneys": {
        "Enable": false,
        "MoneyType": "LLMoney", // LLMonet / ScoreBoard
        "MoneyName": "money",
        "ScoreName": ""
    },
    "switchDim": {
        "overWorld": [
            -89.56292724609375, 72.62001037597656, -164.71534729003906
        ], // 切换维度时传送的坐标 xyz(可在游戏中设置)
        "plotWorld": [
            0.7177982926368713, 2.1200098991394043, 0.3800940215587616
        ]
    },
    "plotWorld": {
        "maxBuyPlotCount": 10, // 最大可购买地皮数量
        "buyPlotPrice": 10000, // 购买地皮价格
        "inPlotCanFly": true, // 地皮内可飞行
        "spawnMob": false // 是否生成生物
    }
}
```

## 指令

-   /plo go plot 进入地皮世界
-   /plo go overworld 返回主世界
-   /plo op 地皮世界管理员
-   /plo deop 取消地皮世界管理员
-   /plo plot 打开地皮管理 GUI
-   /plo 打开地皮世界主菜单

## 权限

地皮插件划分 4 个权限等级：

0. 无权限
1. 地皮共享者(被信任的玩家)
2. 地皮所有者
3. 地皮管理员
