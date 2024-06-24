/// <reference path="D:/HelperLib/src/index.d.ts" />

const { PLAPI, PlotPos } = require("./PlotCraft/lib/PLAPI.js");
// const { PLAPI, PlotPos } = require("../lib/PLAPI.js");

logger.info("PlotCeaft-Fixer loading...");
logger.info("Author: engsr6982");

mc.listen(
    "onEntityExplode",
    (source, pos, radius, maxResistance, isDestroy, isFire) => {
        if (isDestroy || isFire) {
            const pps = PLAPI.getPlotPosByPos(pos);
            if (!pps.isValid()) {
                // 地皮外的爆炸
                return false;
            }
        }
    }
);

mc.listen("onDropItem", (player, item) => {
    const pps = PLAPI.getPlotPosByPos(player.pos);
    const level = PLAPI.getPlayerPermission(player.uuid, pps.getPlotID);

    if (pps.isValid()) {
        if (level == 0) return false; // 地皮内
    }
});
