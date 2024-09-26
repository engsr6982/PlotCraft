#include "Global.h"

namespace plo::gui {

void PluginSettingGUI(Player& player) {
    auto* impl = &data::PlotDBStorage::getInstance();
    auto* cfg  = &Config::cfg.switchDim;

    if (!impl->isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "你没有权限执行此操作");
        return;
    }

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 插件设置");

    fm.appendButton("设置当前位置为主世界安全坐标", "textures/ui/Wrenches1", "path", [cfg](Player& pl) {
        if (pl.getDimensionId() != 0) {
            sendText<LogLevel::Error>(pl, "你必须在主世界才能执行此操作");
            return;
        }
        auto const ps     = pl.getPosition();
        cfg->overWorld[0] = ps.x;
        cfg->overWorld[1] = ps.y;
        cfg->overWorld[2] = ps.z;
        Config::updateConfig();
        sendText(pl, "设置成功");
    });

    fm.appendButton("设置当前位置为地皮世界安全坐标", "textures/ui/Wrenches1", "path", [cfg](Player& pl) {
        if (pl.getDimensionId() != getPlotDimensionId()) {
            sendText<LogLevel::Error>(pl, "你必须在地皮世界才能执行此操作");
            return;
        }
        auto const ps     = pl.getPosition();
        cfg->plotWorld[0] = ps.x;
        cfg->plotWorld[1] = ps.y;
        cfg->plotWorld[2] = ps.z;
        Config::updateConfig();
        sendText(pl, "设置成功");
    });

    fm.sendTo(player);
}


} // namespace plo::gui