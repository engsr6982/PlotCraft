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
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/gui/Global.h"
#include "plotcraft/utils/Area.h"
#include "plotcraft/utils/Text.h"


#include "plotcraft/core/CoreUtils.h"

namespace plo::command {

using namespace plo::utils;

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
                sendText<Level::Error>(output, "玩家 \"{}\" 已经是管理员!", param.name);
            } else {
                if (impl.addAdmin(uuid)) {
                    sendText<Level::Success>(output, "成功将玩家 \"{}\" 设为管理员!", param.name);
                } else {
                    sendText<Level::Error>(output, "设置玩家 \"{}\" 为管理员失败!", param.name);
                }
            }
        } else if (param.op == ParamOp::Deop) {
            if (!impl.isAdmin(uuid)) {
                sendText<Level::Error>(output, "玩家 \"{}\" 不是管理员!", param.name);
            } else {
                if (impl.delAdmin(uuid)) {
                    sendText<Level::Success>(output, "成功将玩家 \"{}\" 取消管理员权限!", param.name);
                } else {
                    sendText<Level::Error>(output, "取消玩家 \"{}\" 管理员权限失败!", param.name);
                }
            }
        }
    } else {
        sendText<Level::Error>(output, "获取玩家 \"{}\" UUID失败!", param.name);
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
        player.teleport(Vec3{sw.plotWorld[0], sw.plotWorld[1], sw.plotWorld[2]}, core_utils::getPlotDimensionId());
    }
};
#endif


const auto LambdaPlot = [](CommandOrigin const& origin, CommandOutput& output) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (player.getDimensionId() != core_utils::getPlotDimensionId()) {
        sendText<utils::Level::Error>(player, "此命令只能在地皮世界使用!");
        return;
    }

    PlotPos pos = PlotPos{player.getPosition()};
    if (pos.isValid()) {
        std::shared_ptr<data::PlotMetadata> plot = data::PlotBDStorage::getInstance().getPlot(pos.getPlotID());
        if (plot == nullptr) {
            plot = data::PlotMetadata::make(pos.getPlotID(), pos.x, pos.z);
        }

        gui::PlotGUI(player, plot, false);
    } else {
        sendText<Level::Error>(output, "无效的地皮坐标!");
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
    sendText<Level::Success>(output, "操作完成!");
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


bool registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo", "PlotCraft");


    cmd.overload<ParamOp>().required("op").required("name").execute(LambdaOP); // plo <op|deop> <name>
    cmd.overload().text("db").text("save").execute(LambdaDBSave);              // plo db save
    cmd.overload().text("this").execute(LambdaPlot);                           // plo this
    cmd.overload().text("buy").execute(LambdaPlot);                            // plo buy
    cmd.overload().text("mgr").execute(LambdaMgr);                             // plo mgr
    cmd.overload().text("setting").execute(LambdaSetting);                     // plo setting
    cmd.overload().execute(LambdaDefault);                                     // plo

#ifndef OVERWORLD
    cmd.overload<ParamGo>().text("go").required("dim").execute(LambdaGo); // plo go <overworld|plot>
#endif

    return true;
}

} // namespace plo::command