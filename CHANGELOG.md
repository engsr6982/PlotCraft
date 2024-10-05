# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2024-10-?

### Added

- 支持地皮合并
- 新增 `PlotRoad`、`PlotCross`、`Polygon` 类

### Changed

- 重构 `EconomySystem` 类
- 重构 `PlotShopGUI` 逻辑
- 重构表单 `_addSharePlayer`

## [1.0.2] - 2024-8-28

### Fixed

- 修复地皮出售用户输入价格过大导致崩溃
- 修复部分获取地皮指针未判空
- 修复玩家无权限下可放置矿车、盔甲架

## [1.0.1] - 2024-8-25

### Fixed

- 修复 UseFrameBlockCallback、SpawnProjectileCallback 意外拦截其他维度事件

## [1.0.0] - 2024-8-23

### Added

- 新增 “模板生成器” 和 “模板记录器”
- Config.generator 新增 `type`、`templateFile` 配置
- 新增 Radius、Cube 类

### Changed

- 重构 EconomicSystem、PlotPos
- Config.moneys 改为 Config.economy
- GUI 支持地皮管理员操作
- 地皮世界填充平原群系
- 重构权限系统（细分权限）

### Removed

- 移除 RemoteCall API
- 移除 Config.plotWorld.eventListener.onUseItemOnWhiteList

## [0.6.1] - 2024-07-24

### Changed

- 适配 LeviLamina 0.13.4

## [0.6.0] - 2024-07-23

### Added:

- 配置文件 `plotWorld` 新增 `eventListener` 配置
- 配置文件 `eventListener` 新增 `onSculkSpreadListener` 事件开关
- 配置文件 `eventListener` 新增 `onSculkBlockGrowthListener` 事件开关
- 配置文件 `eventListener` 新增 `onUseItemOnWhiteList` 配置
- 事件系统 新增 `SculkBlockGrowthEvent`、`SculkSpreadEvent` 事件
- RemoteCall 新增 `PlotMetadata.getPermissionTableConst` API

### Changed

- 命令 `/plo plot` 更改为 `/plo this`
- 命令新增 `setting`、`mgr` 重载
- 支持自定义地皮权限（对访客）

### Fixed

- 修复 地皮边框含水 Bug [#5](https://github.com/engsr6982/PlotCraft/issues/5)
- 修复 在地皮边框放置方块 坐标计算错误

## [0.5.2] - 2024-07-16

### Fixed

- 修复 PlotAdmins 数据解析错误

## [0.5.1] - 2024-07-15

### Fixed

- 修复地皮管理员权限异常
- 修复 `PlotMetadata` 潜在的生命周期问题
- 修复 `PlayerLeavePlot` 事件异常
- 修复地皮飞行功能异常

## [0.5.0] - 2024-07-11

### Changed

- 重构数据库

### Added

- 命令新增 `/plo buy`、`/plo db save` 重载
- 支持关闭底部地皮提示

## [0.4.2] - 2024-07-05

### Fixed

- 修复 `PlayerLeavePlot` 事件异常触发

## [0.4.1] - 2024-07-03

### Changed

- 调整 PlayerAttackEvent 权限处理
- PlotCraft-Fixer.js 新增 `onUseFrameBlock、onPistonTryPush、onMobHurt` 事件处理

## [0.4.0] - 2024-07-03

### Added

- 玩家出售地皮支持扣除税率

### Changed

- 重构 Event.cc 实现
- 表单添加 Image

### Fixed

- 修复 [#3](https://github.com/engsr6982/PlotCraft/issues/3)
- 修复 `updateConfig` 引起的崩溃

## [0.3.0] - 2024-06-26

### Added

- PLAPI.js 新增 `getDimidFromString`
- 配置文件支持限制地皮传送维度

### Changed

- 修改 PLAPI.js `PlotPos.getPlotID` 实现

### Fixed

- 修复未创建`data`文件夹导致 SQL 加载失败

## [0.2.0] - 2024-06-25

### Added

- 新增地皮传送功能
- PLEvent 新增 `PlayerEnterPlot`、`PlayerLeavePlot` 事件

### Changed

- SDK 导出 `EconomyQueue` 类
- 调整 RemoteCall API 导出时机
- 调整 PLAPI.js `PlotPos` 构造函数参数

## [0.1.2] - 2024-06-25

### Changed

- 修改未启用经济时的经济消耗提示
- 禁止在主世界购买地皮

### Fixed

- 修复开启经济系统但无效果 bug

## [0.1.1] - 2024-06-24

## [0.1.0] - 2024-06-24
