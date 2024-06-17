class PlotPos {
  /**
   * 地皮坐标类
   * 请不要直接new此类，请从PLApi.getPlotPosByFlotPos或PLApi.getPlotPosByXZ获取实例
   * @param {number} x 地皮X坐标
   * @param {number} z 地皮Z坐标
   * @param {FloatPos} min 地皮小端点坐标
   * @param {FloatPos} max 地皮大端点坐标
   * @param {boolean} isValid 地皮是否有效
   */
  constructor(x, z, min, max, isValid) {
    this.x = x;
    this.z = z;
    this.min = min;
    this.max = max;
    this.mIsValid = isValid;
  }

  getMin() {
    return this.min;
  }
  getMax() {
    return this.max;
  }
  isValid() {
    return this.mIsValid;
  }
  toString() {}
  toDebug() {}
}

class PLApi {
  static getPlotPosByFlotPos(pos) {}
  static getPlotPosByXZ(x, z) {}
}
