#include "Global.h"


namespace plo::gui {


void PlotShareGUI(Player& player, std::shared_ptr<PlotMetadata> pt) {
    auto* ndb = &PlayerNameDB::getInstance();

    SimpleForm fm{PLUGIN_TITLE};
    fm.setContent("地皮共享设置\n将玩家设置为当前地皮共享者(信任者)后\n被授权的玩家拥有共享的地皮权限(放置、破坏、修改)"
                  "。注意：此权限不包含打开GUI修改地皮的权限。");

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { PlotGUI(pl, pt, true); });

    fm.appendButton("清除所有共享者", "textures/ui/recap_glyph_color_2x", "path", [pt](Player& pl) {
        bool const ok = pt->resetSharedPlayers();
        if (ok) sendText(pl, "共享信息已清除");
        else sendText<utils::Level::Error>(pl, "共享信息清除失败");
    });

    fm.appendButton("添加共享者", "textures/ui/color_plus", "path", [pt](Player& pl) { _addSharePlayer(pl, pt); });


    auto sharedInfos = pt->getSharedPlayers();
    for (auto const& si : sharedInfos) {
        fm.appendButton(
            fmt::format("{}\n{}", ndb->getPlayerName(si.mSharedPlayer), si.mSharedTime),
            [pt, si, ndb](Player& pl) {
                ModalForm{
                    PLUGIN_TITLE,
                    fmt::format("共享者: {}\n共享时间: {}", ndb->getPlayerName(si.mSharedPlayer), si.mSharedTime),
                    "删除此玩家的共享权限",
                    "返回"
                }
                    .sendTo(pl, [pt, si, ndb](Player& pl, ModalFormResult const& dt, FormCancelReason) {
                        if (!dt) {
                            sendText(pl, "表单已放弃");
                            return;
                        }
                        if (!(bool)dt.value()) {
                            PlotGUI(pl, pt, true);
                            return;
                        }

                        bool const ok = pt->delSharedPlayer(si.mSharedPlayer);
                        if (ok) PlotShareGUI(pl, pt);
                        else
                            sendText<utils::Level::Error>(
                                pl,
                                "删除玩家 {} 的共享权限失败",
                                ndb->getPlayerName(si.mSharedPlayer)
                            );
                    });
            }
        );
    }

    fm.sendTo(player);
}

void _addSharePlayer(Player& player, std::shared_ptr<PlotMetadata> pt) {
    auto* ndb = &PlayerNameDB::getInstance();

    CustomForm fm{PLUGIN_TITLE};

    std::vector<string> names;
    ll::service::getLevel()->forEachPlayer([&names, &player](Player& p) {
        if (p == player) return true; // 排除自己
        names.push_back(p.getRealName());
        return true;
    });

    fm.appendDropdown("sp", "[在线] 选择共享者:", names);

    fm.appendInput("in", "[离线] 输入共享者的名字:", "string");

    fm.appendToggle("sw", "在线 <-> 离线");

    fm.sendTo(player, [pt, ndb](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        bool const   sw = std::get<uint64_t>(dt->at("sw"));
        string const sp = std::get<string>(dt->at("sp"));
        string const in = std::get<string>(dt->at("in"));

        bool ok = false;

        if (sw) {
            // 离线
            if (in.empty()) {
                sendText(pl, "输入共享者的名字不能为空");
                return;
            }

            auto const uuid = ndb->getPlayerUUID(in);
            if (uuid.empty()) {
                sendText(pl, "输入的共享者不存在,获取UUID失败");
                return;
            }

            ok = pt->addSharedPlayer(uuid);
        } else {
            // 在线
            ok = pt->addSharedPlayer(ndb->getPlayerUUID(sp));
        }

        if (ok) sendText(pl, "共享权限添加成功");
        else sendText<utils::Level::Error>(pl, "共享权限添加失败");
    });
}


} // namespace plo::gui