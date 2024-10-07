# PlotCraft

基于 LeviLamina & MoreDimension 开发，适用于 BDS(Bedrock Server)的地皮插件(系统)！

- [x] 独立维度 / 主世界维度 自由选择
- [x] 可自定义地皮大小
- [x] 可自定义地皮方块
- [x] 玩家地皮出售系统
- [x] 玩家地皮评论系统
- [x] 玩家自定义设置
- [x] 地皮传送
- [x] 地皮平原群系
- [x] 地皮模板系统
  - [x] 模板生成器
    - 按模板生成地皮
  - [x] 模板记录器
    - 制作地皮模板
- [x] 地皮合并

## 安装

### Lip

```bash
lip install github.com/engsr6982/PlotCraft
```

### 手动安装

我们不推荐这么做，因为处理不好依赖版本可能导致未知的问题  
如果您需要手动安装 PlotCraft，您需要安装以下前置组件（插件）

- MoreDimension
- LegacyMoney

> Tip  
> 编译目标 Overwload 为 true 时 MoreDimension 无需安装  
> 插件默认版本是依赖上述组件，特殊版本需自行从源编译

## 命令

插件安装完毕后，启动服务器（BDS），进入服务器  
传送至对应维度，插件将会生成地皮地形

PlotCraft 注册了以下命令：

```command
/plo go plot        进入地皮世界
/plo go overworld   返回主世界
/plo op <name>      添加地皮管理员
/plo deop <name>    移除地皮管理员
/plo this           打开脚下地皮的管理 GUI
/plo mgr            打开插件设置GUI
/plo setting        打开玩家设置GUI
/plo                打开地皮系统主菜单
/plo db save        立即保存数据到数据库
/plo buy            购买脚下地皮（出售状态）

地皮模板记录命令(此功能请查看下文 "地皮模板系统")
/plo template record execute <fileName: string>
/plo template record pos1
/plo template record pos2
/plo template record reset
/plo template record start <starty: int> <endy: int> <roadwidth: int> <fillBedrock: Boolean> <defaultBlock: Block>
```

## 地皮模板系统

PlotCraft 提供了地皮模板功能，您可以通过模板生成地皮，也可以将地皮制作成模板。

模板系统包括：

- 模板生成器
  - 模板生成器可以将模板文件生成地皮，您需要先制作好模板文件，然后使用模板生成器生成地皮
- 模板记录器
  - 模板记录器可以将地皮制作成模板，您需要先选择好地皮，然后使用模板记录器制作模板

详细使用方法请参考 [模板使用文档](./docs/TemplateSystem.md)

## 开发 & 扩展

PlotCraft 提供 SDK 包，您可以通过 SDK 包扩展 PlotCraft 的功能

SDK 包可在 Release 页面下载

您也可以通过 xmake 来管理依赖

```lua
add_repositories("engsr6982-repo https://github.com/engsr6982/xmake-repo.git")

add_requires("plotcraft")

package("xxx")
    -- ...
    add_packages("plotcraft")
```

## 配置文件

```json
{
  "version": 7,
  "generator": {
    // 注意:
    // 请确定好生成器配置再进入地皮世界
    // 地皮世界生成地形后，请不要再修改生成器配置，否则会出现已生成的区块和未生成区块之间前街错误（地形错误）
    // 如果修改生成器导致的区块衔接错误，请使用地图编辑器删除错位的区块（Amulet）

    "type": "Default", // 生成器类型 Default 或 Template

    // Default 生成器配置:
    "plotWidth": 64, // 地皮大小（正方形）
    "roadWidth": 5, // 道路宽度
    "subChunkNum": 4, // 子区块数量(生成的子区块数量，4 * 16 = 64 即世界高度为 0 )
    "roadBlock": "minecraft:cherry_planks", // 道路方块
    "fillBlock": "minecraft:grass_block", // 填充方块
    "borderBlock": "minecraft:stone_block_slab", // 边界方块

    // Template 生成器配置:
    "templateFile": "TestTemplate.json" // 模板文件名，模板文件必须放置在 config 目录下
  },
  "economy": {
    "enabled": false, // 是否启用经济系统
    "kit": "LegacyMoney", // 经济系统类型 LegacyMoney / ScoreBoard (如果设置 LegacyMoney 则需要安装 LegacyMoney)
    "scoreboard": "", // 计分板名称
    "currency": "金币" // 经济名称
  },
  "plotWorld": {
    "maxBuyPlotCount": 25, // 玩家最大持有地皮数量
    "buyPlotPrice": 1000, // 购买地皮价格
    "inPlotCanFly": true, // 是否启用地皮飞行
    "playerSellPlotTax": 0.1, // 玩家出售地皮税率
    "spawnMob": false // 地皮世界是否生成实体
  },
  "switchDim": {
    // 地皮维度和主世界切换传送坐标，此项可在游戏中设置
    "overWorld": [0.0, 100.0, 0.0],
    "plotWorld": [0.5, 0.0, 0.5]
  },
  "allowedPlotTeleportDim": [
    // 允许从以下维度传送到地皮
    0, 1, 2, 3
  ]
}
```
