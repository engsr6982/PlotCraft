#include "Command.h"


namespace plo::command {

/*
namespace PlotMergeBindData {

std::unordered_map<string, std::pair<PlotPos, PlotPos>> mBindData; // key: realName

bool isStarted(Player& player) { return mBindData.find(player.getRealName()) != mBindData.end(); }

const auto merge = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::PlotMergeGUI(player);
};

const auto start = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (isStarted(player)) {
        sendText<LogLevel::Error>(output, "请先完成当前操作!");
        return;
    }
    auto sou                                = PlotPos(player.getPosition());
    mBindData[string(player.getRealName())] = std::make_pair(sou, PlotPos());

    sendText(player, "地皮合并已开启，前往目标地皮使用命令 /plo merge target 选择目标地皮");
    sendText(player, "已自动选择当前位置为源地皮，如需修改使用 /plo merge source");
};

const auto source = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (!isStarted(player)) {
        sendText<LogLevel::Error>(output, "请先使用 /plo merge start 开启地皮合并功能!");
        return;
    }
    auto sou = PlotPos(player.getPosition());

    mBindData[player.getRealName()].first = sou;

    sendText(player, "已更改源地皮为: {}", sou.toString());
};

const auto target = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (!isStarted(player)) {
        sendText<LogLevel::Error>(output, "请先使用 /plo merge start 开启地皮合并功能!");
        return;
    }
    auto& dt = mBindData[player.getRealName()];

    dt.second = PlotPos(player.getPosition());

    sendText(player, "已选择目标地皮: {}", dt.second.toString());
    sendText(player, "使用 /plo merge confirm 确认合并");
};

const auto confirm = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (!isStarted(player)) {
        sendText<LogLevel::Error>(output, "请先使用 /plo merge start 开启地皮合并功能!");
        return;
    }

    auto& dt = mBindData[player.getRealName()];
    if (!dt.first.isValid() || !dt.second.isValid()) {
        sendText<LogLevel::Error>(output, "源地皮或目标地皮无效，请重新选择");
        return;
    }

    auto firID = dt.first.getPlotID();
    auto secID = dt.second.getPlotID();

    if (!PlotPos::isAdjacent(dt.first, dt.second)) {
        sendText<LogLevel::Error>(output, "{} 和 {} 不是相邻地皮", firID, secID);
        return;
    }

    auto& db      = data::PlotDBStorage::getInstance();
    auto  firMeta = db.getPlot(firID);
    auto  secMeta = db.getPlot(secID);

    if (!firMeta || !secMeta) {
        sendText<LogLevel::Error>(output, "源地皮或目标地皮无主，请重新选择");
        return;
    }

    auto uuid = player.getUuid().asString();
    if (!firMeta->isOwner(uuid) || !secMeta->isOwner(uuid)) {
        sendText<LogLevel::Error>(output, "您不是源地皮或目标地皮的主人，请重新选择");
        return;
    }

    if (db.isMergedPlot(firID)) {
        firID = db.getOwnerPlotID(firID);
    }


    const bool ok = db.tryMergePlot(dt.first, dt.second);
};

const auto cancel = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (!isStarted(player)) {
        sendText<LogLevel::Error>(output, "您未开启地皮合并功能，无需取消");
        return;
    }
    mBindData.erase(player.getRealName());
    sendText(player, "操作已取消");
};

}; // namespace PlotMergeBindData
 */

void _SetUpMergeCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(COMMAND_NAME, "PlotCraft");

    // cmd.overload().text("merge").execute(PlotMergeBindData::merge);                   // plo merge
    // cmd.overload().text("merge").text("start").execute(PlotMergeBindData::start);     // plo merge start
    // cmd.overload().text("merge").text("source").execute(PlotMergeBindData::source);   // plo merge source
    // cmd.overload().text("merge").text("target").execute(PlotMergeBindData::target);   // plo merge target
    // cmd.overload().text("merge").text("confirm").execute(PlotMergeBindData::confirm); // plo merge confirm
    // cmd.overload().text("merge").text("cancel").execute(PlotMergeBindData::cancel);   // plo merge cancel

    // plo


}


} // namespace plo::command
