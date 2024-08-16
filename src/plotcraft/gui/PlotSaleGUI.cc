#include "Global.h"


namespace plo::gui {


void PlotSaleGUI(Player& player, PlotMetadataPtr pt) {
    bool const isSaleing = pt->isSale();

    SimpleForm fm{PLUGIN_TITLE};

    if (isSaleing) {
        fm.setContent(fmt::format("你正在出售地皮 {}，价格为 {}。", pt->getPlotID(), pt->getSalePrice()));

        fm.appendButton("编辑出售价格", "textures/ui/book_edit_default", "path", [pt](Player& pl) {
            _sellPlotAndEditPrice(pl, pt, true);
        });
        fm.appendButton("取消出售", "textures/ui/cancel", "path", [pt](Player& pl) {
            bool const ok = pt->setSaleStatus(false, 0);
            if (ok) sendText(pl, "出售已取消");
            else sendText<LogLevel::Error>(pl, "出售取消失败");
        });
    } else {
        fm.setContent("此地皮没有出售，无法查询出售信息。");

        fm.appendButton("出售当前地皮", "textures/ui/icon_minecoin_9x9", "path", [pt](Player& pl) {
            _sellPlotAndEditPrice(pl, pt, false);
        });
    }

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { PlotGUI(pl, pt, true); });

    fm.sendTo(player);
}


void _sellPlotAndEditPrice(Player& player, PlotMetadataPtr pt, bool edit) {
    CustomForm fm{PLUGIN_TITLE};

    fm.appendLabel("地皮确认出售后，其它玩家可以查看到你的出售信息。当玩家购买后，你的地皮将被转移到购买者的名下("
                   "自动重置共享信息)。");

    if (edit) fm.appendInput("pr", "请输入出售价格:", "integer", std::to_string(pt->getSalePrice()));
    else fm.appendInput("pr", "请输入出售价格:", "integer");

    fm.sendTo(player, [pt, edit](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        string const pr = std::get<string>(dt->at("pr")); // 输入框只能输入字符串

        if (!std::regex_match(pr, std::regex("^\\d+$"))) {
            sendText<LogLevel::Error>(pl, "价格必须为整数");
            return;
        }

        int const p = std::stoi(pr);
        if (p <= 0) {
            sendText<LogLevel::Error>(pl, "价格必须大于0");
            return;
        }

        if (edit) {
            bool const ok = pt->setSalePrice(p);
            if (ok) sendText(pl, "出售价格已修改");
            else sendText<LogLevel::Error>(pl, "出售价格修改失败");
        } else {
            bool const ok = pt->setSaleStatus(true, p);
            if (ok) sendText(pl, "出售成功");
            else sendText<LogLevel::Error>(pl, "出售失败");
        }
    });
}


} // namespace plo::gui