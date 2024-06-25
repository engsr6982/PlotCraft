/// <reference path="D:/HelperLib/src/index.d.ts" />

const CallEvent = ll.imports("PLAPI", "CallPLEvent");

let nextID = 0;
function getNextFuncName() {
  return `PLEvent_${nextID++}`;
}

class PLEvent {
  constructor() {
    throw new Error("Static class cannot be instantiated.");
  }

  /**
   * @param {string} eventName
   * @param {Function} callback
   */
  static on(eventName, callback) {
    const funcName = getNextFuncName();
    ll.exports(callback, eventName, funcName);
    const ok = CallEvent(eventName, funcName);
    if (!ok) {
      logger.error(`Unknow event name: ${eventName}`);
    }
    return ok;
  }
}

module.exports = {
  PLEvent,
};
