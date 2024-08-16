#include "plugin/MyPlugin.h"

#include <corecrt.h>
#include <filesystem>
#include <memory>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerLevel.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOutputType.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/ServerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/command/Command.h"
#include "plotcraft/core/PlotDimension.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/event/Event.h"
#include "plotcraft/utils/EconomySystem.h"
#include "plotcraft/utils/Mc.h"


#if !defined(OVERWORLD)
#include "more_dimensions/api/dimension/CustomDimensionManager.h"
#endif


namespace fs = std::filesystem;

namespace my_plugin {

static std::unique_ptr<MyPlugin> instance;

MyPlugin& MyPlugin::getInstance() { return *instance; }

bool MyPlugin::load() {
    auto& logger = getSelf().getLogger();
    logger.info(R"(                                                           )");
    logger.info(R"(         ____   __        __   ______              ____ __ )");
    logger.info(R"(        / __ \ / /____   / /_ / ____/_____ ____ _ / __// /_)");
    logger.info(R"(       / /_/ // // __ \ / __// /    / ___// __ `// /_ / __/)");
    logger.info(R"(      / ____// // /_/ // /_ / /___ / /   / /_/ // __// /_  )");
    logger.info(R"(     /_/    /_/ \____/ \__/ \____//_/    \__,_//_/   \__/  )");
    logger.info(R"(                                                           )");
    logger.info(R"(                 ---- Author: engsr6982 ----               )");
    logger.info(R"(                                                           )");
    logger.info("加载中...");

    logger.info("编译版本信息: {}", BuildVersionInfo);

    logger.info("尝试创建必要的文件夹...");

    if (!fs::exists(getSelf().getDataDir())) {
        fs::create_directories(getSelf().getDataDir());
    }

    logger.info("尝试加载数据...");
    plo::config::loadConfig();
    ll::i18n::load(getSelf().getLangDir());
    plo::data::PlotBDStorage::getInstance().load();
    plo::data::PlayerNameDB::getInstance().initPlayerNameDB();
    plo::EconomyQueue::getInstance().load();

    plo::utils::EconomySystem::getInstance().updateConfig(plo::config::cfg.economy);

    return true;
}


bool MyPlugin::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("Enabling...");
    logger.info("尝试注册 命令、维度、事件...");

#ifdef DEBUG
    plo::mc::executeCommand("gamerule showcoordinates true");
#endif

#if !defined(OVERWORLD)
    more_dimensions::CustomDimensionManager::getInstance().addDimension<plo::core::PlotDimension>("plot");
#endif

    plo::event::registerEventListener();                          // 注册事件监听器
    plo::command::registerCommand();                              // 注册命令
    plo::data::PlotBDStorage::getInstance().tryStartSaveThread(); // 尝试启动自动保存线程

    return true;
}

bool MyPlugin::disable() {
    auto& logger = getSelf().getLogger();
    logger.info("Disabling...");

    logger.warn("正在保存数据，请不要强制关闭进程...");
    plo::data::PlotBDStorage::getInstance().save();

    plo::event::unRegisterEventListener();

    return true;
}

} // namespace my_plugin

LL_REGISTER_MOD(my_plugin::MyPlugin, my_plugin::instance);
