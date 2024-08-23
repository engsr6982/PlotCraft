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

    if (cfg.generator.subChunkNum <= 0 && cfg.generator.type == PlotGeneratorType::Default) {
        cfg.generator.subChunkNum = 1;
        logger.error("subChunkNum 不能小于等于0，已自动设置为1");
        updateConfig();
    }

    if (cfg.generator.templateFile == "" && cfg.generator.type == PlotGeneratorType::Template) {
        throw std::runtime_error("初始化失败，templateFile 不能为空!");
    }

    if (!ok) {
        logger.warn("loadConfig 返回了预期之外的结果，尝试备份并覆写配置文件...");
        updateConfig();
    }
}


void updateConfig() {
    fs::path dir    = my_plugin::MyPlugin::getInstance().getSelf().getConfigDir();
    fs::path source = dir / "Config.json";
    fs::path target = dir / "Config.json.bak";

    try {
        fs::copy(source, target, fs::copy_options::overwrite_existing);

        ll::config::saveConfig(cfg, source);
    } catch (std::exception const& e) {
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error("更新配置文件失败: {}", e.what());
    }
}

// double calculateMergePlotPrice(int mergeCount) {
//     if (mergeCount <= 0) {
//         return cfg.plotWorld.baseMergePlotPrice;
//     }
//     double multiplier = std::max(1.0, cfg.plotWorld.mergePriceMultiplier);
//     return cfg.plotWorld.baseMergePlotPrice * std::pow(multiplier, mergeCount);
// }


} // namespace plo::config