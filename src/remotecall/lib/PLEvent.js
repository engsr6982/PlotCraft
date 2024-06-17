/**
 * @param {string} event_name 事件名称
 * @param {string} export_name 导出名称
 * @returns {boolean} 是否成功
 */
const call = ll.imports("PLAPI", "PLEvent_Call");

let id = 0;

/**
 * @returns {string}
 */
function getID() {
  return `PLEvent_${id++}`;
}

class PLEvent {
  /**
   * 监听 PlotCraft 地皮事件
   * @param {string} event
   * @param {function} callback
   * @returns {boolean} 是否成功
   */
  static on(event, callback) {
    const id = getID(); // 生成唯一的ID
    ll.exports(callback, event, id); // 导出此回调 event为命名空间，id为函数名
    const result = call(event, id); // 通知C++层导入此回调函数
    if (!result) {
      logger.fatal(`PLEvent.on ${event} not found`);
    }
    return result;
  }
}

module.exports = {
  PLEvent,
};
