#include "Global.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include <variant>


namespace plot::gui {


void PlotShareGUI(Player& player, PlotMetadataPtr pt) {
    auto* ndb = &PlayerNameDB::getInstance();

    SimpleForm fm{PLUGIN_TITLE};
    fm.setContent("地皮共享设置\n将玩家设置为当前地皮共享者(信任者)后\n被授权的玩家拥有共享的地皮权限(放置、破坏、修改)"
                  "。注意：此权限不包含打开GUI修改地皮的权限。");

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { PlotGUI(pl, pt, true); });

    fm.appendButton("清除所有共享者", "textures/ui/recap_glyph_color_2x", "path", [pt](Player& pl) {
        bool const ok = pt->resetSharedPlayers();
        if (ok) sendText(pl, "共享信息已清除");
        else sendText<LogLevel::Error>(pl, "共享信息清除失败");
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
                            sendText<LogLevel::Error>(
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

void _addSharePlayer(Player& player, PlotMetadataPtr pt) {
    auto* ndb = &PlayerNameDB::getInstance();

    CustomForm fm{PLUGIN_TITLE};

    std::vector<string> names;
    ll::service::getLevel()->forEachPlayer([&names, &player](Player& p) {
        // if (p == player) return true; // 排除自己
        names.push_back(p.getRealName());
        return true;
    });

    fm.appendDropdown("choose_player", "[在线] 选择共享者:", names);

    fm.appendInput("input_player", "[离线] 输入共享者的名字:", "string");

    fm.appendToggle("switch_online_offline", "在线 <-> 离线");

    fm.sendTo(player, [pt, ndb](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        DebugFormPrint(dt);

        bool const switch_online_offline = std::get<uint64_t>(dt->at("switch_online_offline")); // false: 在线true: 离线

        string realName;
        if (switch_online_offline) {
            // Offline
            auto iter = dt->find("input_player");
            if (iter == dt->end()) {
                sendText<LogLevel::Error>(pl, "表单数据错误 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            if (std::holds_alternative<std::monostate>(iter->second)) {
                sendText(pl, "表单数据错误，Value为空 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            if (!std::holds_alternative<string>(iter->second)) {
                sendText(pl, "表单数据错误, 获取 string 失败 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            realName = std::get<string>(iter->second);

        } else {
            // Online
            auto iter = dt->find("choose_player");
            if (iter == dt->end()) {
                sendText<LogLevel::Error>(pl, "表单数据错误 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            if (std::holds_alternative<std::monostate>(iter->second)) {
                sendText(pl, "表单数据错误，Value为空 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            if (!std::holds_alternative<string>(iter->second)) {
                sendText(pl, "表单数据错误, 获取 string 失败 [{}#{}L]", __FILE__, __LINE__);
                return;
            }
            realName = std::get<string>(iter->second);
        }

        if (realName.empty()) {
            sendText<LogLevel::Error>(pl, "玩家名字不能为空");
            return;
        }

        if (realName == pl.getRealName() && !PlotDBStorage::getInstance().isAdmin(pl.getUuid().asString())) {
            sendText<LogLevel::Error>(pl, "您不能将自己添加到共享成员中");
            return;
        }

        string uuid = ndb->getPlayerUUID(realName);
        if (uuid.empty()) {
            sendText<LogLevel::Error>(pl, "获取玩家 {} 的UUID失败", realName);
            return;
        }

        if (pt->addSharedPlayer(uuid)) {
            sendText(pl, "成功添加 {} 为共享成员", realName);
        } else {
            sendText<LogLevel::Error>(pl, "无法添加 {} 为共享成员", realName);
        }
    });
}


} // namespace plot::gui