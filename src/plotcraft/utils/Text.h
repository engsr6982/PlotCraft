#pragma once
#include "Utils.h"
#include "ll/api/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include <mc/server/commands/CommandOutput.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

using string = std::string;

namespace plo::utils {

enum class Level : int { Normal = -1, Debug = 0, Info = 1, Warn = 2, Error = 3, Fatal = 4, Success = 5 };

inline static std::unordered_map<Level, string> Color = {
    {Level::Normal,  "§b"}, // aqua
    {Level::Debug,   "§7"}, // gray
    {Level::Info,    "§r"}, // default
    {Level::Warn,    "§e"}, // yellow
    {Level::Error,   "§c"}, // red
    {Level::Fatal,   "§4"}, // dark_red
    {Level::Success, "§a"}  // green
};

// Template function sendText, usage: sendText() or sendText<MsgLevel::Success>().
template <Level type = Level::Normal, typename... Args>
inline void sendText(Player& player, const string& fmt, Args&&... args) {
    player.sendMessage(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
}
template <Level type = Level::Normal, typename... Args>
inline void sendText(CommandOutput& output, const string& fmt, Args&&... args) {
    if constexpr (type == Level::Error || type == Level::Fatal) {
        output.error(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    } else {
        output.success(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    }
}
template <Level type = Level::Normal, typename... Args>
inline void sendText(Player* player, const string& fmt, Args&&... args) {
    if (player) {
        return sendText<type>(*player, fmt, args...);
    } else {
        std::runtime_error("Failed in sendText: player is nullptr");
    }
}
template <Level type = Level::Normal, typename... Args>
inline void sendText(const string& realName, const string& fmt, Args&&... args) {
    auto level = ll::service::getLevel();
    if (level.has_value()) {
        return sendText<type>(level->getPlayer(realName), fmt, args...);
    } else {
        std::runtime_error("Failed in sendText: level is nullptr");
    }
}
template <Level type = Level::Normal, typename... Args>
inline void sendText(ll::Logger& logger, const string& fmt, Args&&... args) {
    if constexpr (type == Level::Error) {
        logger.error(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    } else if constexpr (type == Level::Fatal) {
        logger.fatal(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    } else if constexpr (type == Level::Warn) {
        logger.warn(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    } else {
        logger.info(fmt::format(PLUGIN_TITLE + Color[type] + fmt, args...));
    }
}

} // namespace plo::utils