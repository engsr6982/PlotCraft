#include "Global.h"

namespace plo::gui {


void PlayerSettingGUI(Player& player) {
    CustomForm fm{PLUGIN_TITLE};

    auto setting     = data::PlotDBStorage::getInstance().getPlayerSetting(player.getUuid().asString());
    auto i18n        = ll::i18n::getInstance().get();
    auto settingJson = JsonHelper::structToJson(setting);

    for (auto const& [key, value] : settingJson.items()) {
        if (key == "version") continue;
        fm.appendToggle(key, string(i18n->get(key)), value.get<bool>());
    }

    fm.sendTo(player, [setting, settingJson](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        utils::DebugFormPrint(dt);

        json              setj = settingJson; // copy
        PlayerSettingItem it   = setting;

        for (auto const& [key, value] : setj.items()) {
            if (key == "version") continue;
            bool const val = std::get<uint64_t>(dt->at(key));
            setj[key]      = val;
        }

        JsonHelper::jsonToStruct(setj, it);

        data::PlotDBStorage::getInstance().setPlayerSetting(pl.getUuid().asString(), it);
        sendText(pl, "设置成功");
    });
}


} // namespace plo::gui