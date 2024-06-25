/// <reference path="D:/HelperLib/src/index.d.ts" />

// 开发环境
// const { PLAPI, PlotPos } = require("../lib/PLAPI.js");
// const { PLEvent } = require("../lib/PLEvent.js");

// 生产环境
const { PLAPI, PlotPos } = require("./PlotCraft/lib/PLAPI.js");
const { PLEvent } = require("./PlotCraft/lib/PLEvent.js");

logger.info("PlotCeaft-Fixer loading...");
logger.info("Author: engsr6982");

mc.listen(
  "onEntityExplode",
  (source, pos, radius, maxResistance, isDestroy, isFire) => {
    logger.debug("onEntityExplode");
    if (pos.dimid != PLAPI.getPlotWorldDimid()) return;
    if (isDestroy || isFire) {
      return false;
    }
  }
);

// 以下为 地皮插件自定义事件测试代码

// PLEvent.on(
//   "PlayerEnterPlot",
//   /**
//    * @param {Player} player
//    * @param {Array<number>} PlotPosConstructorArgs
//    */
//   (player, PlotPosConstructorArgs) => {
//     // 使用事件提供的参数构造 PlotPos 对象
//     // 受 RemoteCall 限制，只能导出参数在Js构造虚对象
//     const pps = new PlotPos(PlotPosConstructorArgs);
//     logger.info(`玩家: ${player.realName} 进入地皮: ${pps.toString()}`);
//   }
// );

// PLEvent.on(
//   "PlayerLeavePlot",
//   /**
//    * @param {Player} player
//    * @param {Array<number>} PlotPosConstructorArgs
//    */
//   (player, PlotPosConstructorArgs) => {
//     const pps = new PlotPos(PlotPosConstructorArgs);
//     logger.info(`玩家: ${player.realName} 离开地皮: ${pps.toString()}`);
//   }
// );
