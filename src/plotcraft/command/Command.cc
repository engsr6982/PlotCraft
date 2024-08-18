#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/math/Vec3.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandPositionFloat.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/util/FeatureTerrainAdjustmentsUtil.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/core/Utils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/gui/Global.h"
#include <unordered_map>


namespace plo::command {
using namespace plo::utils;
using namespace plo::mc;

struct ParamOp {
    enum OperationOP { Op, Deop } op;
    string name;
};
const auto LambdaOP = [](CommandOrigin const& origin, CommandOutput& output, ParamOp const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::DedicatedServer);
    auto& pdb  = data::PlayerNameDB::getInstance();
    auto  uuid = pdb.getPlayerUUID(param.name);
    if (!uuid.empty()) {
        auto& impl = data::PlotBDStorage::getInstance();
        if (param.op == ParamOp::Op) {
            if (impl.isAdmin(uuid)) {
                sendText<LogLevel::Error>(output, "玩家 \"{}\" 已经是管理员!", param.name);
            } else {
                if (impl.addAdmin(uuid)) {
                    sendText<LogLevel::Success>(output, "成功将玩家 \"{}\" 设为管理员!", param.name);
                } else {
                    sendText<LogLevel::Error>(output, "设置玩家 \"{}\" 为管理员失败!", param.name);
                }
            }
        } else if (param.op == ParamOp::Deop) {
            if (!impl.isAdmin(uuid)) {
                sendText<LogLevel::Error>(output, "玩家 \"{}\" 不是管理员!", param.name);
            } else {
                if (impl.delAdmin(uuid)) {
                    sendText<LogLevel::Success>(output, "成功将玩家 \"{}\" 取消管理员权限!", param.name);
                } else {
                    sendText<LogLevel::Error>(output, "取消玩家 \"{}\" 管理员权限失败!", param.name);
                }
            }
        }
    } else {
        sendText<LogLevel::Error>(output, "获取玩家 \"{}\" UUID失败!", param.name);
    }
};


#ifndef OVERWORLD
struct ParamGo {
    enum GoDimension { overworld, plot } dim;
};
const auto LambdaGo = [](CommandOrigin const& origin, CommandOutput& output, ParamGo const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());

    auto& sw = config::cfg.switchDim;

    if (param.dim == ParamGo::overworld) {
        player.teleport(Vec3{sw.overWorld[0], sw.overWorld[1], sw.overWorld[2]}, 0); // 传送到重生点
    } else {
        player.teleport(Vec3{sw.plotWorld[0], sw.plotWorld[1], sw.plotWorld[2]}, core::getPlotDimensionId());
    }
};
#endif


const auto LambdaPlot = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (player.getDimensionId() != core::getPlotDimensionId()) {
        sendText<LogLevel::Error>(player, "此命令只能在地皮世界使用!");
        return;
    }

    PlotPos pos = PlotPos{player.getPosition()};
    if (pos.isValid()) {
        std::shared_ptr<data::PlotMetadata> plot = data::PlotBDStorage::getInstance().getPlot(pos.getPlotID());
        if (plot == nullptr) {
            plot = data::PlotMetadata::make(pos.getPlotID(), pos.mX, pos.mZ);
        }

        gui::PlotGUI(player, plot, false);
    } else {
        sendText<LogLevel::Error>(output, "无效的地皮坐标!");
    }
};


const auto LambdaDefault = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::MainGUI(player);
};


const auto LambdaDBSave = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::DedicatedServer);
    data::PlotBDStorage::getInstance().save();
    sendText<LogLevel::Success>(output, "操作完成!");
};

const auto LambdaMgr = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::PluginSettingGUI(player);
};
const auto LambdaSetting = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::PlayerSettingGUI(player);
};

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

    auto& db      = data::PlotBDStorage::getInstance();
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

bool registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo", "PlotCraft");


    cmd.overload<ParamOp>().required("op").required("name").execute(LambdaOP); // plo <op|deop> <name>
    cmd.overload().text("db").text("save").execute(LambdaDBSave);              // plo db save
    cmd.overload().text("this").execute(LambdaPlot);                           // plo this
    cmd.overload().text("buy").execute(LambdaPlot);                            // plo buy
    cmd.overload().text("mgr").execute(LambdaMgr);                             // plo mgr
    cmd.overload().text("setting").execute(LambdaSetting);                     // plo setting
    cmd.overload().execute(LambdaDefault);                                     // plo

    // cmd.overload().text("merge").execute(PlotMergeBindData::merge);                   // plo merge
    // cmd.overload().text("merge").text("start").execute(PlotMergeBindData::start);     // plo merge start
    // cmd.overload().text("merge").text("source").execute(PlotMergeBindData::source);   // plo merge source
    // cmd.overload().text("merge").text("target").execute(PlotMergeBindData::target);   // plo merge target
    // cmd.overload().text("merge").text("confirm").execute(PlotMergeBindData::confirm); // plo merge confirm
    // cmd.overload().text("merge").text("cancel").execute(PlotMergeBindData::cancel);   // plo merge cancel

    _setupTemplateCommand();

#ifndef OVERWORLD
    cmd.overload<ParamGo>().text("go").required("dim").execute(LambdaGo); // plo go <overworld|plot>
#endif

    return true;
}

} // namespace plo::command