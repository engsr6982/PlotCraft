/// <reference path="D:/HelperLib/src/index.d.ts" />

const { PLAPI, PlotPos } = require("./PlotCraft/lib/PLAPI.js");
// const { PLAPI, PlotPos } = require("../lib/PLAPI.js");

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
