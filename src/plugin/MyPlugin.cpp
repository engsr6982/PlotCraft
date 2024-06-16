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

#include "plotcraft/config/Config.h"
#include "plotcraft/core/PlotDimension.h"
#include "plotcraft/database/DataBase.h"
#include "plotcraft/event/Event.h"


namespace my_plugin {

static std::unique_ptr<MyPlugin> instance;

MyPlugin& MyPlugin::getInstance() { return *instance; }

bool MyPlugin::load() {
    auto& logger = getSelf().getLogger();
    logger.info("Loading...");

#ifdef DEBUG
    logger.consoleLevel = 5; // 调试模式下输出详细日志
#endif

    plo::config::loadConfig();
    plo::database::PlayerNameDB::getInstance().initPlayerNameDB();
    plo::database::PlotDB::getInstance().load();

    return true;
}

bool MyPlugin::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("Enabling...");

    // 注册自定义维度
    logger.info("Registering plot dimension...");
    more_dimensions::CustomDimensionManager::getInstance().addDimension<plo::core::PlotDimension>("plot");

    plo::event::registerEventListener();

#ifdef DEBUG
    CommandContext ctx = CommandContext(
        "gamerule showcoordinates true",
        std::make_unique<ServerCommandOrigin>(
            "Server",
            ll::service::getLevel()->asServer(),
            CommandPermissionLevel::Owner,
            0
        )
    );
    ll::service::getMinecraft()->getCommands().executeCommand(ctx, false);
#endif

    return true;
}

bool MyPlugin::disable() {
    getSelf().getLogger().info("Disabling...");

    plo::event::unRegisterEventListener();

    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::instance);
