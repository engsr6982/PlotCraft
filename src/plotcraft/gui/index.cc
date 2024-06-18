#include "index.h"
#include "ll/api/form/SimpleForm.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/dimension/VanillaDimensions.h"


using namespace ll::form;
using namespace plo::utils;
using namespace plo::database;

namespace plo::gui {

void index(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 选择一个操作:");

    if (player.getDimensionId() == VanillaDimensions::fromString("plot")) {
        fm.appendButton("前往主世界", [](Player& player) { mc::executeCommand("plo go overworld", &player); });
    } else {
        fm.appendButton("前往地皮世界", [](Player& player) { mc::executeCommand("plo go plot", &player); });
    }


    fm.appendButton("地皮管理", [](Player& player) {});

    fm.appendButton("插件管理", [](Player& player) {});


    fm.appendButton("退出", [](Player&) {});
    fm.sendTo(player);
}


void plot(Player& player, PlotPos plotPos) {
    // TODO: 实现
}


} // namespace plo::gui