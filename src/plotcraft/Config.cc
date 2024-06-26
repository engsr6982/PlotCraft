#include "plotcraft/Config.h"
#include "ll/api/Config.h"
#include "plotcraft/utils/Date.h"
#include "plugin/MyPlugin.h"
#include <filesystem>
#include <stdexcept>
#include <string>


namespace fs = std::filesystem;

namespace plo::config {

_Config cfg;

void loadConfig() {
    auto& mSelf  = my_plugin::MyPlugin::getInstance().getSelf();
    auto& logger = mSelf.getLogger();

    fs::path path = mSelf.getConfigDir() / "Config.json";

    bool const ok = ll::config::loadConfig(cfg, path);

    if (!ok) {
        logger.warn("loadConfig 返回了预期之外的结果，尝试备份并覆写配置文件...");
        updateConfig();
    }


    if (cfg.generator.subChunkNum <= 0) {
        cfg.generator.subChunkNum = 1;
        logger.warn("subChunkNum 不能小于等于0，已自动设置为1");
        updateConfig();
    }
}


void updateConfig() {
    fs::path dir = my_plugin::MyPlugin::getInstance().getSelf().getConfigDir();

    fs::copy(dir / "Config.json", dir / ("Config.json.bak"));

    ll::config::saveConfig(cfg, dir / "Config.json");
}


} // namespace plo::config