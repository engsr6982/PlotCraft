/// <reference path="D:/HelperLib/src/index.d.ts" />
/// <reference path="./Type.d.ts" />

// 使用:
// let { PLAPI } = require("./PlotCraft/lib/PLAPI.js");
// PLAPI.getPlotWorldDimid();
//
// 注意：
// 本文件仅提供部分基础API。
// 如果你需要更强大API，请使用SDK，或者自行export提交pr

const _Remote_ = {
  getPlotWorldDimid: ll.imports("PLAPI", "getPlotWorldDimid"),
  getDimidFromString: ll.imports("PLAPI", "getDimidFromString"),
  PlotPos_toString: ll.imports("PLAPI", "PlotPos_toString"),
  PlotPos_toDebug: ll.imports("PLAPI", "PlotPos_toDebug"),
  PlotPos_isPosInPlot: ll.imports("PLAPI", "PlotPos_isPosInPlot"),
  PlotPos_getSafestPos: ll.imports("PLAPI", "PlotPos_getSafestPos"),
  PlotPos_isPosOnBorder: ll.imports("PLAPI", "PlotPos_isPosOnBorder"),
  PlotPos_getPlotID: ll.imports("PLAPI", "PlotPos_getPlotID"),
  getPlayerPermission: ll.imports("PLAPI", "getPlayerPermission"),
  PlotPos_getMin: ll.imports("PLAPI", "PlotPos_getMin"),
  PlotPos_getMax: ll.imports("PLAPI", "PlotPos_getMax"),
  getPlotPosByPos: ll.imports("PLAPI", "getPlotPosByPos"),
  getPlotPosByXZ: ll.imports("PLAPI", "getPlotPosByXZ"),
  PlotMetadata_getPermissionTableConst: ll.imports(
    "PLAPI",
    "PlotMetadata_getPermissionTableConst"
  ),
};

class PLAPI {
  /**
   * 获取地皮世界维度 ID
   * @returns {number}
   */
  static getPlotWorldDimid() {
    return _Remote_.getPlotWorldDimid();
  }

  /**
   * 使用名称获取维度id
   * @param {string} name
   */
  static getDimidFromString(name) {
    return _Remote_.getDimidFromString(name);
  }

  /**
   * @param {number} x
   * @param {number} y
   * @param {number} z
   * @returns {FloatPos}
   */
  static getPlotWorldFloatPos(x, y, z) {
    return new FloatPos(x, y, z, this.getPlotWorldDimid());
  }

  /**
   * 获取玩家权限
   * @param {string} uuid 玩家 UUID
   * @param {string} plotID 地皮 ID
   * @param {boolean} ignoreAdmin 忽略管理员
   * @returns {0|1|2|3} 无权限 | 共享者 | 所有者 | 管理员
   */
  static getPlayerPermission(uuid, plotID, ignoreAdmin = false) {
    return _Remote_.getPlayerPermission(uuid, plotID, ignoreAdmin);
  }

  /**
   * 获取 PlotPos 对象
   * @param {FloatPos} pos
   * @returns {PlotPos}
   */
  static getPlotPosByPos(pos) {
    const val = _Remote_.getPlotPosByPos(pos);
    return new PlotPos(val);
  }

  /**
   * 获取 PlotPos 对象
   * @param {number} x
   * @param {number} z
   * @returns {PlotPos}
   */
  static getPlotPosByXZ(x, z) {
    const val = _Remote_.getPlotPosByXZ(x, z);
    return new PlotPos(val);
  }
}

class PlotPos {
  /**
   * @param {Array<number>} constructorArgs 构造参数
   */
  constructor(constructorArgs) {
    if (constructorArgs.length !== 3) {
      throw new Error(
        "PlotPos constructor args length error, Please from PLAPI.getPlotPosByPos or PLAPI.getPlotPosByXZ"
      );
    }
    this.x = constructorArgs[0]; // x
    this.z = constructorArgs[1]; // z
    this.mIsVaild = constructorArgs[2]; // 是否有效
  }

  /**
   * @returns {boolean}
   */
  isValid() {
    return this.mIsVaild;
  }

  /**
   * @returns {FloatPos}
   */
  getMin() {
    return _Remote_.PlotPos_getMin(this.x, this.z);
  }

  /**
   * @returns {FloatPos}
   */
  getMax() {
    return _Remote_.PlotPos_getMax(this.x, this.z);
  }

  /**
   * @returns {string}
   */
  toString() {
    return _Remote_.PlotPos_toString(this.x, this.z);
  }

  /**
   * @returns {string}
   */
  getPlotID() {
    return _Remote_.PlotPos_getPlotID(this.x, this.z);
  }

  /**
   * @returns {string}
   */
  toDebug() {
    return _Remote_.PlotPos_toDebug(this.x, this.z);
  }

  /**
   * @param {FloatPos} pos
   * @returns {boolean}
   */
  isPosInPlot(pos) {
    return _Remote_.PlotPos_isPosInPlot(this.x, this.z, pos);
  }

  /**
   * @returns {FloatPos}
   */
  getSafestPos() {
    return _Remote_.PlotPos_getSafestPos(this.x, this.z);
  }

  /**
   * @param {FloatPos} pos
   * @returns {boolean}
   */
  isPosOnBorder(pos) {
    return _Remote_.PlotPos_isPosOnBorder(this.x, this.z, pos);
  }
}

class PlotMetadata {
  constructor(plotID) {
    this.mPlotID = plotID;
  }

  getPermissionTableConst() {
    let dt = _Remote_.PlotMetadata_getPermissionTableConst(this.mPlotID);
    if (dt == "") return null;
    return JSON.parse(dt);
  }
}

module.exports = {
  PLAPI,
  PlotPos,
  PlotMetadata,
};
