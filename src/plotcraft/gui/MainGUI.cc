#include "Global.h"

namespace plot::gui {


void MainGUI(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 选择一个操作:");

    if (player.getDimensionId().id == getPlotWorldDimensionId()) {
        fm.appendButton("前往主世界", "textures/ui/realmsIcon", "path", [](Player& pl) {
            mc::executeCommand("plot go overworld", &pl);
        });
    } else {
        fm.appendButton("前往地皮世界", "textures/ui/realmsIcon", "path", [](Player& pl) {
            mc::executeCommand("plot go plot", &pl);
        });
    }

    fm.appendButton("管理脚下地皮", "textures/ui/icon_recipe_item", "path", [](Player& pl) {
        mc::executeCommand("plot this", &pl);
    });

    fm.appendButton("管理地皮", "textures/ui/icon_recipe_nature", "path", [](Player& pl) { _selectPlot(pl); });

    fm.appendButton("地皮商店", "textures/ui/store_home_icon", "path", [](Player& pl) { PlotShopGUI(pl); });

    fm.appendButton("自定义设置", "textures/ui/gear", "path", [](Player& pl) { PlayerSettingGUI(pl); });
    fm.appendButton("插件设置\n(管理员)", "textures/ui/gear", "path", [](Player& pl) { PluginSettingGUI(pl); });

    fm.appendButton("退出", [](Player&) {});
    fm.sendTo(player);
}


} // namespace plot::gui