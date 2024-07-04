declare class PLAPI {
  /**
   * 获取地皮世界的维度ID
   */
  static getPlotWorldDimid(): number;

  /**
   * 通过维度名称获取维度ID
   * @param name 维度名称
   */
  static getDimidFromString(name: string): number;

  /**
   * 获取地皮世界的坐标
   * @param x x坐标
   * @param y y坐标
   * @param z z坐标
   */
  static getPlotWorldFloatPos(x: number, y: number, z: number): FloatPos;

  /**
   * 获取玩家权限
   * @param uuid 玩家UUID
   * @param plotID 地皮ID
   * @param ignoreAdmin 是否忽略管理员权限
   * @param ignoreCache 是否忽略缓存
   */
  static getPlayerPermission(
    uuid: string,
    plotID: string,
    ignoreAdmin: boolean,
    ignoreCache: boolean
  ): PlotPermission;

  /**
   * 通过坐标对象获取地皮坐标对象
   * @param pos 浮点坐标
   */
  static getPlotPosByPos(pos: FloatPos): PlotPos;

  /**
   * 通过地皮坐标获取坐标对象
   * @param x x坐标
   * @param z z坐标
   */
  static getPlotPosByXZ(x: number, z: number): PlotPos;
}

enum PlotPermission {
  None = 0,
  Shared = 1,
  Owner = 2,
  Admin = 3,
}

declare class PlotPos {
  constructor(constructorArgs: [3, number, number, number]);

  isValid(): boolean;

  getMin(): FloatPos;

  getMax(): FloatPos;

  toString(): string;

  getPlotID(): string;

  toDebug(): string;

  isPosInPlot(pos: FloatPos): boolean;

  getSafestPos(): FloatPos;

  isPosOnBorder(pos: FloatPos): boolean;
}
