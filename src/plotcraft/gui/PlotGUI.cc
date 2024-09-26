#include "Global.h"


namespace plo::gui {


void _selectPlot(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { MainGUI(pl); });


    auto plots = data::PlotDBStorage::getInstance().getPlots();

    for (auto const& plt : plots) {
        if (!plt->isOwner(player.getUuid().asString())) continue; // 不是自己的地皮
        fm.appendButton(fmt::format("{}\n{}", plt->getPlotName(), plt->getPlotID()), [plt](Player& pl) {
            PlotGUI(pl, plt, true);
        });
    }

    fm.sendTo(player);
}


void PlotGUI(Player& player, PlotMetadataPtr pt, bool ret) {
    SimpleForm fm{PLUGIN_TITLE};

    auto& ndb = PlayerNameDB::getInstance();
    auto& cfg = Config::cfg;

    bool const hasOwner       = !pt->getPlotOwner().empty();                                   // 是否有主人
    bool const hasSale        = pt->isSale();                                                  // 是否出售
    bool const isOwner        = hasOwner && player.getUuid().asString() == pt->getPlotOwner(); // 是否是主人
    bool const isSharedMember = pt->isSharedPlayer(player.getUuid().asString()); // 是否是地皮共享成员
    bool const isAdmin        = PlotDBStorage::getInstance().isAdmin(player.getUuid().asString()); // 是否是管理员

    fm.setContent(fmt::format(
        "地皮 {} 的元数据:\n地皮主人: {}\n地皮名称: {}\n是否出售: {}\n出售价格: {}\n  ",
        pt->getPlotID(),
        !hasOwner ? "无主" : ndb.getPlayerName(pt->getPlotOwner()),
        pt->getPlotName(),
        hasOwner ? hasSale ? "是" : "否" : "是",
        hasOwner ? hasSale ? std::to_string(pt->getSalePrice()) : "null"
                 : std::to_string(Config::cfg.plotWorld.buyPlotPrice)
    ));


    if ((!hasOwner || hasSale) && !isOwner)
        fm.appendButton("购买地皮", "textures/ui/confirm", "path", [pt](Player& pl) { _buyPlot(pl, pt); });

    if (((isOwner || isSharedMember) && utils::some(cfg.allowedPlotTeleportDim, player.getDimensionId().id)) || isAdmin)
        fm.appendButton("传送到此地皮", "textures/ui/move", "path", [pt](Player& pl) {
            auto const v3 = PPos{pt->getX(), pt->getZ()}.getSafestPos();
            pl.teleport(v3, getPlotDimensionId());
            sendText(pl, "传送成功");
        });

    if (isOwner || isAdmin) {
        fm.appendButton("权限管理", "textures/ui/gear", "path", [pt](Player& pl) { PlotPermissionGUI(pl, pt); });

        fm.appendButton("修改地皮名称", "textures/ui/book_edit_default", "path", [pt](Player& pl) {
            _changePlotName(pl, pt);
        });

        fm.appendButton("地皮出售", "textures/ui/MCoin", "path", [pt](Player& pl) { PlotSaleGUI(pl, pt); });

        fm.appendButton("共享地皮", "textures/ui/share_microsoft", "path", [pt](Player& pl) { PlotShareGUI(pl, pt); });
    }

    if (hasOwner)
        fm.appendButton("地皮评论", "textures/ui/icon_sign", "path", [pt](Player& pl) { PlotCommentGUI(pl, pt); });

    if (ret) fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { _selectPlot(pl); });

    fm.sendTo(player);
}


void _changePlotName(Player& player, PlotMetadataPtr pt) {
    auto* bus = &ll::event::EventBus::getInstance();

    pev::PlayerChangePlotNameBefore ev{&player, pt};
    bus->publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("pn", "地皮名称:", "string", pt->getPlotName());

    fm.sendTo(player, [pt, bus](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        string pn = std::get<string>(dt->at("pn"));

        if (pn.empty()) {
            sendText(pl, "地皮名称不能为空");
            return;
        }

        bool const ok = pt->setPlotName(pn);

        if (ok) {
            pev::PlayerChangePlotNameAfter ev{&pl, pt, pn};
            bus->publish(ev);

            sendText(pl, "地皮名称已修改");
        } else {
            sendText<LogLevel::Error>(pl, "地皮名称修改失败");
        }
    });
}


} // namespace plo::gui