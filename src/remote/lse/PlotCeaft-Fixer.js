/// <reference path="D:/HelperLib/src/index.d.ts" />
/// <reference path="../lib/Type.d.ts" />

const { PLAPI, PlotPos } = require("./PlotCraft/lib/PLAPI.js");
const { PLEvent } = require("./PlotCraft/lib/PLEvent.js");

logger.info("PlotCeaft-Fixer loading...");
logger.info("Author: engsr6982");

// 实体爆炸
mc.listen(
  "onEntityExplode",
  (source, pos, radius, maxResistance, isDestroy, isFire) => {
    logger.debug("onEntityExplode");
    if (pos.dimid != PLAPI.getPlotWorldDimid()) return;
    if (isDestroy || isFire) return false;
  }
);

// 耕地退化
mc.listen("onFarmLandDecay", (pos, ent) => {
  logger.debug("onFarmLandDecay");
  if (pos.dimid != PLAPI.getPlotWorldDimid()) return; // 仅处理 PlotWorld 内的事件
  const pps = PLAPI.getPlotPosByPos(pos);

  if (ent.isPlayer()) {
    const pl = ent.toPlayer();
    const level = PLAPI.getPlayerPermission(pl.uuid, pps.getPlotID());
    if (pps.isValid() && level == 0) return false; // 地皮内 & 无权限 => 拦截
  } else return false; // 非玩家 => 拦截
});

// 玩家操作展示框
mc.listen("onUseFrameBlock", (pl, bl) => {
  const pos = bl.pos;
  if (pos.dimid != PLAPI.getPlotWorldDimid()) return;

  const pps = PLAPI.getPlotPosByPos(pos);
  const lv = PLAPI.getPlayerPermission(pl.uuid, pps.getPlotID());

  if (lv == 0) return false; // 无权限 => 拦截
});

// 活塞尝试推动方块
mc.listen("onPistonTryPush", (pistionPos, bl) => {
  if (pistionPos.dimid != PLAPI.getPlotWorldDimid()) return;

  const pushedBlockPos = bl.pos; // 尝试推动的方块位置

  const sou = PLAPI.getPlotPosByPos(pistionPos);
  const tar = PLAPI.getPlotPosByPos(pushedBlockPos);

  if (sou.isValid() && tar.isValid() && !tar.isPosOnBorder(pushedBlockPos))
    return;

  if (!sou.isValid() && !tar.isValid() && !tar.isPosOnBorder(pushedBlockPos))
    return;

  return false; // 拦截
});

// 生物受伤
mc.listen("onMobHurt", (mob, sou, dmg, cause) => {
  const mobPos = mob.pos;
  if (mobPos.dimid != PLAPI.getPlotWorldDimid()) return;
  if (!sou || !sou.isPlayer()) return;

  const player = sou.toPlayer();
  const pps = PLAPI.getPlotPosByPos(player.pos);
  const lv = PLAPI.getPlayerPermission(player.uuid, pps.getPlotID());

  if (pps.isValid() && lv == 0) return false; // 地皮内 & 无权限 => 拦截
});

// ======================================================================
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
