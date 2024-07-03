# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.0] - 2024-07-03

### Changed

- 重构 Event.cc 实现

### Fixed

- 修复 [#3](https://github.com/engsr6982/PlotCraft/issues/3)

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
