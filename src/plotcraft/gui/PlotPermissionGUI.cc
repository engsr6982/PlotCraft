#include "Global.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/i18n/I18n.h"


namespace plot::gui {


void PlotPermissionGUI(Player& player, PlotMetadataPtr pt) {
    auto& i18n                = ll::i18n::getInstance();
    auto  permissionTableJson = JsonHelper::structToJson(pt->getPermissionTable());

    CustomForm fm{PLUGIN_TITLE};

    for (auto& [key, value] : permissionTableJson.items()) {
        fm.appendToggle(string(key), string(i18n.get(key, ll::i18n::getDefaultLocaleCode())), value.get<bool>());
    }

    fm.sendTo(player, [pt, permissionTableJson](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        utils::DebugFormPrint(dt);

        json setj = permissionTableJson; // copy
        for (auto const& [key, value] : setj.items()) {
            bool const val = std::get<uint64_t>(dt->at(key));
            setj[key]      = val;
        }

        JsonHelper::jsonToStructNoMerge(setj, pt->mPermissionTable); // 反射回成员
        sendText(pl, "权限表已更新");
    });
}


} // namespace plot::gui