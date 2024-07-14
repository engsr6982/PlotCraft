# PlotCraft

基于 LeviLamina 开发，适用于 BDS(Bedrock Server)的地皮插件！

- 地皮维度/主世界维度 自由选择
- 可自定义地皮大小
- 可自定义方块类型
- 玩家地皮出售系统
- 玩家地皮评论系统
- 玩家自定义设置
- ...

## 安装/使用/开发

### 命令安装

推荐使用 Lip 一键安装（自动处理依赖）
```bash
lip install github.com/engsr6982/PlotCraft
```

### 手动安装

我们不推荐这么做，因为处理不好依赖版本可能导致未知的问题  
如果您需要手动安装 PlotCraft，您需要安装以下前置组件（插件）
- MoreDimension
- LegacyMoney
- LegacyRemoteCall

> Tip  
> 编译目标为 Overwload 时 MoreDimension 无需安装  
> 未编译RemoteCall相关代码时，LegacyRemoteCall 为可选  
> 插件默认版本是依赖上述组件，特殊版本需自行从源编译

### 使用

插件安装完毕后，启动服务器（BDS），进入服务器  
传送至对应维度，插件将会生成地皮地形

PlotCraft 注册了以下命令：

```command
/plo go plot        进入地皮世界
/plo go overworld   返回主世界
/plo op <name>      添加地皮管理员
/plo deop <name>    移除地皮管理员
/plo plot           打开脚下地皮的管理 GUI
/plo                打开地皮系统主菜单
/plo db save        立即保存数据到数据库
/plo buy            购买脚下地皮（出售状态）
```

> [warning]  
> 插件压缩包内 lse 文件夹中，附带了一个Js文件 `PlotCraft-Fixer.js`  
> 此文件用于修复插件本体未处理的事件，如果需要使用请将其放入`plugins/`目录下, 由 LSE（LegacyScriptEngine） 引擎加载。

### 开发 & 扩展 & 贡献

PlotCraft 提供 SDK 和 RemoteCall API。

#### SDK 开发

使用 SDK 的优势在于，您能访问 PlotCraft 几乎全部的 API，使用C++开发

SDK 可在 Release 页面下载


#### RemoteCall 开发

RemoteCall 为 LSE 引擎提供的远程调用 API，提供基础的交互能力。  
但受限于 RemoteCall，仅能访问 PlotCraft 部分已导出的 API

API声明、封装文件，可在每个 Release 版本压缩包里找到  
开发可参考 PlotCraft-Fixer.js


#### 贡献

我们欢迎您 Pr 本插件，为本插件增加更多功能！(＾ω＾)ﾉ🎉

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
    "overWorld": [-89.56292724609375, 72.62001037597656, -164.71534729003906], // 切换维度时传送的坐标 xyz(可在游戏中设置)
    "plotWorld": [0.7177982926368713, 2.1200098991394043, 0.3800940215587616]
  },
  "plotWorld": {
    "maxBuyPlotCount": 10, // 最大可购买地皮数量
    "buyPlotPrice": 10000, // 购买地皮价格
    "inPlotCanFly": true, // 地皮内可飞行
    "spawnMob": false // 是否生成生物
  }
}
```
