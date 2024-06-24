#include "plugin/MyPlugin.h"

#include <corecrt.h>
#include <memory>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerLevel.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOutputType.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/ServerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "more_dimensions/api/dimension/CustomDimensionManager.h"

#include "plotcraft/Config.h"
#include "plotcraft/DataBase.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/command/Command.h"
#include "plotcraft/core/PlotDimension.h"
#include "plotcraft/event/Event.h"
#include "plotcraft/utils/Mc.h"


namespace my_plugin {

static std::unique_ptr<MyPlugin> instance;

MyPlugin& MyPlugin::getInstance() { return *instance; }

bool MyPlugin::load() {
    auto& logger = getSelf().getLogger();
    logger.info("Loading...");

#ifdef DEBUG
    logger.consoleLevel = 5; // 调试模式下输出详细日志
#endif

    // 初始化数据（非MCAPI）
    logger.info("Try loading config、database...");
    plo::config::loadConfig();
    plo::database::PlayerNameDB::getInstance().initPlayerNameDB();
    plo::database::PlotDB::getInstance().load();
    plo::EconomyQueue::getInstance().load();

    return true;
}


bool MyPlugin::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("Enabling...");

#ifdef DEBUG
    plo::mc::executeCommand("gamerule showcoordinates true");
#endif

    // 注册MCAPI
    logger.info("Try registering command、event listener、dimension...");
    more_dimensions::CustomDimensionManager::getInstance().addDimension<plo::core::PlotDimension>("plot");
    if (!plo::event::registerEventListener()) return false; // 注册事件监听器
    plo::command::registerCommand();                        // 注册命令

    return true;
}

bool MyPlugin::disable() {
    getSelf().getLogger().info("Disabling...");

    plo::event::unRegisterEventListener();

    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::instance);
