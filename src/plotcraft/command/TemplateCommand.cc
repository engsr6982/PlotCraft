#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandSelector.h"
#include "plotcraft/core/TemplateManager.h"
#include "plotcraft/data/PlotDBStorage.h"

namespace plo::command {
using namespace plo::data;
using namespace plo::mc;
using TemplateManager = core::TemplateManager;

struct StartData {
    int              starty;
    int              endy;
    int              roadwidth;
    bool             fillBedrock;
    CommandBlockName defaultBlock;
};
struct ExecuteData {
    std::string fileName;
};


const auto start = [](CommandOrigin const& ori, CommandOutput& out, StartData const& data) {
    CHECK_COMMAND_TYPE(out, ori, CommandOriginType::Player);
    auto& player = *static_cast<Player*>(ori.getEntity());
    if (!PlotDBStorage::getInstance().isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "此命令仅限地皮管理员使用");
        return;
    }
    if (core::TemplateManager::isRecordTemplateing()) {
        sendText<LogLevel::Error>(player, "当前正在记录模板, 请不要重复执行");
        return;
    }

    auto bl = data.defaultBlock.resolveBlock((int)data.defaultBlock.id).getBlock();
    if (bl == nullptr) {
        sendText<LogLevel::Error>(player, "获取默认方块失败");
        return;
    }

    bool ok = TemplateManager::prepareRecordTemplate(
        data.starty,
        data.endy,
        data.roadwidth,
        data.fillBedrock,
        bl->getTypeName()
    );

    if (ok) {
        sendText<LogLevel::Info>(player, "模板记录已开启，请设置起始点pos1和结束点pos2");
    } else {
        sendText<LogLevel::Error>(player, "模板记录开启失败");
    }
};
const auto pos1 = [](CommandOrigin const& ori, CommandOutput& out) {
    CHECK_COMMAND_TYPE(out, ori, CommandOriginType::Player);
    auto& player = *static_cast<Player*>(ori.getEntity());
    if (!PlotDBStorage::getInstance().isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "此命令仅限地皮管理员使用");
        return;
    }
    if (!core::TemplateManager::isRecordTemplateing()) {
        sendText<LogLevel::Error>(player, "当前没有正在记录模板");
        return;
    }

    auto cps = ChunkPos(player.getPosition());
    if (cps.x != 0 && cps.z != 0) {
        sendText<LogLevel::Error>(player, "目前只支持在0,0坐标设置起始点");
        return;
    }

    bool ok = TemplateManager::postRecordTemplateStart(cps);
    if (ok) {
        sendText<LogLevel::Info>(player, "模板记录起始点设置成功");
    } else {
        sendText<LogLevel::Error>(player, "模板记录起始点设置失败");
    }
};
const auto pos2 = [](CommandOrigin const& ori, CommandOutput& out) {
    CHECK_COMMAND_TYPE(out, ori, CommandOriginType::Player);
    auto& player = *static_cast<Player*>(ori.getEntity());
    if (!PlotDBStorage::getInstance().isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "此命令仅限地皮管理员使用");
        return;
    }
    if (!core::TemplateManager::isRecordTemplateing()) {
        sendText<LogLevel::Error>(player, "当前没有正在记录模板");
        return;
    }

    auto cps = ChunkPos(player.getPosition());
    if (cps.x != cps.z) {
        sendText<LogLevel::Error>(player, "仅支持正方形地皮模板");
        return;
    }

    bool ok = TemplateManager::postRecordTemplateEnd(cps);
    if (ok) {
        sendText<LogLevel::Info>(player, "模板记录结束点设置成功");
    } else {
        sendText<LogLevel::Error>(player, "模板记录结束点设置失败");
    }
};

const auto execute = [](CommandOrigin const& ori, CommandOutput& out, ExecuteData const& data) {
    CHECK_COMMAND_TYPE(out, ori, CommandOriginType::Player);
    auto& player = *static_cast<Player*>(ori.getEntity());
    if (!PlotDBStorage::getInstance().isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "此命令仅限地皮管理员使用");
        return;
    }
    if (!core::TemplateManager::isRecordTemplateing()) {
        sendText<LogLevel::Error>(player, "当前没有正在记录模板");
        return;
    }

    if (data.fileName.empty()) {
        sendText<LogLevel::Error>(player, "请输入模板文件名");
        return;
    }

    sendText<LogLevel::Info>(player, "正在记录模板，请稍候...");
    sendText<LogLevel::Warn>(player, "模板记录期间服务端可能未响应，请等待插件处理完成...");

    bool ok = TemplateManager::postRecordAndSaveTemplate(data.fileName, player);
    if (ok) {
        sendText<LogLevel::Info>(player, "模板记录完成");
    } else {
        sendText<LogLevel::Error>(player, "模板记录失败");
    }
};

const auto reset = [](CommandOrigin const& ori, CommandOutput& out) {
    CHECK_COMMAND_TYPE(out, ori, CommandOriginType::Player);
    auto& player = *static_cast<Player*>(ori.getEntity());
    if (!PlotDBStorage::getInstance().isAdmin(player.getUuid().asString())) {
        sendText<LogLevel::Error>(player, "此命令仅限地皮管理员使用");
        return;
    }
    if (!core::TemplateManager::isRecordTemplateing()) {
        sendText<LogLevel::Error>(player, "当前没有正在记录模板");
        return;
    }

    bool ok = TemplateManager::resetRecordTemplate();
    if (ok) {
        sendText<LogLevel::Info>(player, "模板记录已重置");
    } else {
        sendText<LogLevel::Error>(player, "模板记录重置失败");
    }
};


void _setupTemplateCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo");

    // plo template record start <int start> <int end> <int road> <bool fillBedrock> <Block defaultBlock>
    cmd.overload<StartData>()
        .text("template")
        .text("record")
        .text("start")
        .required("starty")
        .required("endy")
        .required("roadwidth")
        .required("fillBedrock")
        .required("defaultBlock")
        .execute(start);

    // plo template record pos1
    cmd.overload().text("template").text("record").text("pos1").execute(pos1);

    // plo template record pos2
    cmd.overload().text("template").text("record").text("pos2").execute(pos2);

    // plo template record execute <string fileName>
    cmd.overload<ExecuteData>().text("template").text("record").text("execute").required("fileName").execute(execute);

    // plo template record reset
    cmd.overload().text("template").text("record").text("reset").execute(reset);
}
} // namespace plo::command