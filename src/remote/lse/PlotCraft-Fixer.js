/// <reference path="D:/HelperLib/src/index.d.ts" />
/// <reference path="../lib/Type.d.ts" />

const { PLAPI, PlotPos, PlotMetadata } = require("./PlotCraft/lib/PLAPI.js");
const { PLEvent } = require("./PlotCraft/lib/PLEvent.js");

logger.info("PlotCeaft-Fixer loading...");
logger.info("Author: engsr6982");

// 实体爆炸
mc.listen(
  "onEntityExplode",
  (source, pos, radius, maxResistance, isDestroy, isFire) => {
    logger.debug("onEntityExplode");
    if (pos.dimid === PLAPI.getPlotWorldDimid()) return false;
  }
);

// 耕地退化
mc.listen("onFarmLandDecay", (pos, ent) => {
  logger.debug("onFarmLandDecay");
  if (pos.dimid != PLAPI.getPlotWorldDimid()) return; // 仅处理 PlotWorld 内的事件
  const pps = PLAPI.getPlotPosByPos(pos);
  const tab = new PlotMetadata(pps.getPlotID).getPermissionTableConst();
  const valid = pps.isValid();

  if (ent.isPlayer()) {
    const pl = ent.toPlayer();
    const lv = PLAPI.getPlayerPermission(pl.uuid, pps.getPlotID());

    if (lv >= 1 && valid) return; // 管理员 & 所有者 & 成员 & 地皮内 => 放行
    if (lv !== 3 && !valid) return false; // 非管理员 & 地皮外 => 拦截
  }

  if (!valid) return false; // 地皮外 => 拦截
  if (!tab) return false; // 全新地皮 => 拦截
  if (!tab.canFarmLandDecay) return false; // 地皮禁止耕地退化 => 拦截
});

// 玩家操作展示框
mc.listen("onUseFrameBlock", (pl, bl) => {
  const pos = bl.pos;
  if (pos.dimid != PLAPI.getPlotWorldDimid()) return;

  const pps = PLAPI.getPlotPosByPos(pos);
  const lv = PLAPI.getPlayerPermission(pl.uuid, pps.getPlotID());
  const valid = pps.isValid();

  if (lv >= 1) return; // 管理员 & 所有者 & 成员 => 放行

  const tab = new PlotMetadata(pps.getPlotID).getPermissionTableConst();
  if (!valid) return false; // 地皮外 => 拦截
  if (!tab) return false; // 全新地皮 => 拦截
  if (!tab.canOperateFrame) return false; // 地皮禁止操作展示框 => 拦截
});

// 活塞尝试推动方块
mc.listen("onPistonTryPush", (pistionPos, bl) => {
  if (pistionPos.dimid != PLAPI.getPlotWorldDimid()) return;

  const pushedBlockPos = bl.pos; // 尝试推动的方块位置

  const sou = PLAPI.getPlotPosByPos(pistionPos);
  const tar = PLAPI.getPlotPosByPos(pushedBlockPos);

  if (sou.isValid() && tar.isValid() && !tar.isPosOnBorder(pushedBlockPos))
    return; // 地皮内 & 地皮外 & 非边框 => 放行

  if (!sou.isValid() && !tar.isValid() && !tar.isPosOnBorder(pushedBlockPos))
    return; // 地皮外 & 地皮外 & 非边框 => 放行

  return false;
});

// 生物受伤
mc.listen("onMobHurt", (mob, sou, dmg, cause) => {
  const mobPos = mob.pos; // 受伤的生物位置
  if (mobPos.dimid != PLAPI.getPlotWorldDimid()) return;

  const pps = PLAPI.getPlotPosByPos(mobPos);
  const tab = new PlotMetadata(pps.getPlotID).getPermissionTableConst();

  const valid = pps.isValid();
  if (!valid) return; // 地皮外 => 放行

  if (sou && sou.isPlayer()) {
    const pl = sou.toPlayer();
    const lv = PLAPI.getPlayerPermission(pl.uuid, pps.getPlotID());

    if (lv >= 1) return; // 管理员 & 所有者 & 成员 => 放行
  }

  if (!tab) return; // 全新地皮 => 放行
  if (!tab.canMobHurt) return false; // 地皮禁止生物受伤 => 拦截
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
