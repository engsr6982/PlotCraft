#include "plotcraft/Config.h"
#include "ll/api/Config.h"
#include "plugin/MyPlugin.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace plo::config {

_Config cfg;

void loadConfig() {
    auto& mSelf  = my_plugin::MyPlugin::getInstance().getSelf();
    auto& logger = mSelf.getLogger();

    fs::path path = mSelf.getConfigDir() / "Config.json";

    bool noNeddReWrite = ll::config::loadConfig(cfg, path);
    if (!noNeddReWrite) {
        logger.warn("配置文件异常，请检查配置文件版本或配置是否正确");
    }
}


void updateConfig() {
    fs::path path = my_plugin::MyPlugin::getInstance().getSelf().getConfigDir() / "Config.json";
    ll::config::saveConfig(cfg, path);
}


} // namespace plo::config