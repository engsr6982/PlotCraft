#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/math/Vec3.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Config.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/gui/Global.h"


namespace plot::command {
using namespace plot::utils;
using namespace plot::mc;

struct ParamOp {
    enum OperationOP { Op, Deop } op;
    string name;
};
const auto LambdaOP = [](CommandOrigin const& origin, CommandOutput& output, ParamOp const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::DedicatedServer);
    auto& pdb  = data::PlayerNameDB::getInstance();
    auto  uuid = pdb.getPlayerUUID(param.name);
    if (!uuid.empty()) {
        auto& impl = data::PlotDBStorage::getInstance();
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
static const auto LambdaGo = [](CommandOrigin const& origin, CommandOutput& output, ParamGo const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());

    auto& sw = Config::cfg.switchDim;

    if (param.dim == ParamGo::overworld) {
        player.teleport(Vec3{sw.overWorld[0], sw.overWorld[1], sw.overWorld[2]}, 0); // 传送到重生点
    } else {
        player.teleport(Vec3{sw.plotWorld[0], sw.plotWorld[1], sw.plotWorld[2]}, getPlotWorldDimensionId());
    }
};
#endif


static const auto LambdaPlot = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (player.getDimensionId() != getPlotWorldDimensionId()) {
        sendText<LogLevel::Error>(player, "此命令只能在地皮世界使用!");
        return;
    }

    PlotPos pos = PlotPos{player.getPosition()};
    if (pos.isValid()) {
        std::shared_ptr<data::PlotMetadata> plot = data::PlotDBStorage::getInstance().getPlot(pos.getPlotID());
        if (plot == nullptr) {
            plot = data::PlotMetadata::make(pos.getPlotID(), pos.mX, pos.mZ);
        }

        gui::PlotGUI(player, plot, false);
    } else {
        sendText<LogLevel::Error>(output, "无效的地皮坐标!");
    }
};


static const auto LambdaDefault = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::MainGUI(player);
};


static const auto LambdaDBSave = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::DedicatedServer);
    data::PlotDBStorage::getInstance().save();
    sendText<LogLevel::Success>(output, "操作完成!");
};

static const auto LambdaMgr = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::PluginSettingGUI(player);
};
static const auto LambdaSetting = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    gui::PlayerSettingGUI(player);
};

static const auto LambdaFindUnownedPlot = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (player.getDimensionId() != getPlotWorldDimensionId()) {
        sendText<LogLevel::Error>(player, "此命令只能在地皮世界使用!");
        return;
    }

    auto result = data::PlotDBStorage::getInstance().findUnownedPlot();
    if (!result) {
        sendText<LogLevel::Error>(player, "插件异常, 未找到无主地皮!");
    }

    player.teleport(result->getSafestPos(), getPlotWorldDimensionId());
};


bool registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(COMMAND_NAME, "PlotCraft");

    cmd.overload<ParamOp>().required("op").required("name").execute(LambdaOP); // plot <op|deop> <name>
    cmd.overload().text("db").text("save").execute(LambdaDBSave);              // plot db save
    cmd.overload().text("this").execute(LambdaPlot);                           // plot this
    cmd.overload().text("buy").execute(LambdaPlot);                            // plot buy
    cmd.overload().text("mgr").execute(LambdaMgr);                             // plot mgr
    cmd.overload().text("setting").execute(LambdaSetting);                     // plot setting
    cmd.overload().execute(LambdaDefault);                                     // plot

    // 查找无主地皮
    // plot find unowned_plot
    cmd.overload().text("find").text("unowned_plot").execute(LambdaFindUnownedPlot);

    _setupTemplateCommand();

    // MergeCommand.cc
    extern void _SetUpMergeCommand();
    _SetUpMergeCommand();

#ifdef DEBUG
    // DebugCommand.cc
    extern void SetupDebugCommand();
    SetupDebugCommand();
#endif

#ifndef OVERWORLD
    cmd.overload<ParamGo>().text("go").required("dim").execute(LambdaGo); // plot go <overworld|plot>
#endif

    return true;
}

} // namespace plot::command