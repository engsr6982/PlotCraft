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
   */
  static getPlayerPermission(
    uuid: string,
    plotID: string,
    ignoreAdmin: boolean
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

declare enum PlotPermission {
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

declare type PlotPermissionTable = {
  /**[LL] 破坏方块*/ canDestroyBlock: boolean;
  /**[LL] 放置方块*/ canPlaceBlock: boolean;
  /**[LL] 使用物品(右键)*/ canUseItemOn: boolean;
  /**[LL] 火焰蔓延*/ canFireSpread: boolean;
  /**[LL] 攻击*/ canAttack: boolean;
  /**[LL] 拾取物品*/ canPickupItem: boolean;
  /**[LL] 与方块交互*/ canInteractBlock: boolean;

  /**[LSE] 耕地退化*/ canFarmLandDecay: boolean;
  /**[LSE] 操作展示框*/ canOperateFrame: boolean;
  /**[LSE] 生物受伤*/ canMobHurt: boolean;
  /**[LSE] 攻击方块*/ canAttackBlock: boolean;
  /**[LSE] 操作盔甲架*/ canOperateArmorStand: boolean;
  /**[LSE] 丢弃物品*/ canDropItem: boolean;
  /**[LSE] 踩压压力板*/ canStepOnPressurePlate: boolean;
  /**[LSE] 骑乘*/ canRide: boolean;
  /**[LSE] 凋零破坏方块*/ canWitherDestroyBlock: boolean;
  /**[LSE] 红石更新*/ canRedStoneUpdate: boolean;
};

/**
 * 地皮元信息类
 */
declare class PlotMetadata {
  mPlotID: string;

  /**
   * 获取地皮元信息
   * @param plotID
   */
  constructor(plotID: string): PlotMetadata;

  /**
   * 获取地皮权限表
   * @returns PlotPermissionTable | null
   */
  getPermissionTableConst(): PlotPermissionTable | null;
}
