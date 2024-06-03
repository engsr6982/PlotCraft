#include "plugin/MyPlugin.h"

#include <memory>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"
#include "more_dimensions/api/dimension/CustomDimensionManager.h"
#include "plotcraft/core/PlotDimension.h"

namespace my_plugin {

static std::unique_ptr<MyPlugin> instance;

MyPlugin& MyPlugin::getInstance() { return *instance; }

bool MyPlugin::load() {
    auto& logger = getSelf().getLogger();
    logger.info("Loading...");

#ifdef DEBUG
    logger.consoleLevel = 5; // 调试模式下输出详细日志
#endif

    return true;
}

bool MyPlugin::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("Enabling...");

    // 注册自定义维度
    logger.info("Registering plot dimension...");
    more_dimensions::CustomDimensionManager::getInstance().addDimension<plotcraft::PlotDimension>("plot");

    return true;
}

bool MyPlugin::disable() {
    getSelf().getLogger().info("Disabling...");
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::instance);
