#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

#include "plotcraft/config/Config.h"
#include "plotcraft/database/DataBase.h"
#include "plotcraft/utils/Text.h"


namespace plo::command {

using namespace plo::utils;

struct ParamOp {
    enum OperationOP { Op, Deop } op;
    string name;
};
const auto LambdaOP = [](CommandOrigin const& origin, CommandOutput& output, ParamOp const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::DedicatedServer);
    auto& pdb  = database::PlayerNameDB::getInstance();
    auto  uuid = pdb.getPlayerUUID(param.name);
    if (uuid.has_value() && !uuid->isEmpty()) {
        auto& impl = database::PlotDB::getInstance().getImpl();
        if (param.op == ParamOp::Op) {
            if (impl.isAdmin(*uuid)) {
                sendText<Level::Error>(output, "玩家 \"{}\" 已经是管理员!", param.name);
            } else {
                if (impl.addAdmin(*uuid)) {
                    sendText<Level::Success>(output, "成功将玩家 \"{}\" 设为管理员!", param.name);
                } else {
                    sendText<Level::Error>(output, "设置玩家 \"{}\" 为管理员失败!", param.name);
                }
            }
        } else if (param.op == ParamOp::Deop) {
            if (!impl.isAdmin(*uuid)) {
                sendText<Level::Error>(output, "玩家 \"{}\" 不是管理员!", param.name);
            } else {
                if (impl.removeAdmin(*uuid)) {
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

struct ParamGo {
    enum GoDimension { overworld, plot } dim;
};
const auto LambdaGo = [](CommandOrigin const& origin, CommandOutput& output, ParamGo const& param) {
    CHECK_COMMAND_TYPE(output, origin, CommandOriginType::Player);
    Player& player = *static_cast<Player*>(origin.getEntity());
    if (param.dim == ParamGo::overworld) {
        auto& dim = player.getDimension();
        auto  sp  = dim.getSpawnPos();
        player.teleport(Vec3{sp.x, dim.getSpawnYPosition(), sp.z}, 0); // 传送到重生点
    } else {
        player.teleport(Vec3{0, config::cfg.generator.generatorY + 2, 0}, VanillaDimensions::fromString("plot"));
    }
};


bool registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo", "PlotCraft");

    // plo <op|deop> <name>
    cmd.overload<ParamOp>().required("op").required("name").execute(LambdaOP);

    // plo go <overworld|plot>
    cmd.overload<ParamGo>().text("go").required("dim").execute(LambdaGo);

    return true;
}
} // namespace plo::command