#include "plotcraft/utils/EconomySystem.h"
#include "LLMoney.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/PlayerScoreSetFunction.h"
#include "mc/world/level/Level.h"
#include "mc/world/scores/ScoreInfo.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Utils.h"
#include <mc/world/actor/player/Player.h>
#include <mc/world/scores/Objective.h>
#include <mc/world/scores/Scoreboard.h>
#include <mc/world/scores/ScoreboardId.h>
#include <memory>
#include <stdexcept>


// disable warning C4244: conversion from 'double' to 'int', possible loss of data
#pragma warning(disable : 4244)

namespace plo::utils {
using ll::i18n_literals::operator""_tr;
using SimpleForm = ll::form::SimpleForm;
using namespace plo::mc;


int ScoreBoard_Get(Player& player, string const& scoreName) {
    Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
    Objective*  obj        = scoreboard.getObjective(scoreName);
    if (!obj) {
        sendText<LogLevel::Error>(player, "[Moneys] 插件错误: 找不到指定的计分板: {}"_tr(scoreName));
        return 0;
    }
    ScoreboardId const& id = scoreboard.getScoreboardId(player);
    if (!id.isValid()) {
        scoreboard.createScoreboardId(player);
    }
    return obj->getPlayerScore(id).mScore;
}

bool ScoreBoard_Set(Player& player, int score, string const& scoreName) {
    Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
    Objective*  obj        = scoreboard.getObjective(scoreName);
    if (!obj) {
        sendText<LogLevel::Error>(player, "[Moneys] 插件错误: 找不到指定的计分板: "_tr(scoreName));
        return false;
    }
    const ScoreboardId& id = scoreboard.getScoreboardId(player);
    if (!id.isValid()) {
        scoreboard.createScoreboardId(player);
    }
    bool isSuccess = false;
    scoreboard.modifyPlayerScore(isSuccess, id, *obj, score, PlayerScoreSetFunction::Set);
    return isSuccess;
}

bool ScoreBoard_Add(Player& player, int score, string const& scoreName) {
    Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
    Objective*  obj        = scoreboard.getObjective(scoreName);
    if (!obj) {
        sendText<LogLevel::Error>(player, "[Moneys] 插件错误: 找不到指定的计分板: "_tr(scoreName));
        return false;
    }
    const ScoreboardId& id = scoreboard.getScoreboardId(player);
    if (!id.isValid()) {
        scoreboard.createScoreboardId(player);
    }
    bool isSuccess = false;
    scoreboard.modifyPlayerScore(isSuccess, id, *obj, score, PlayerScoreSetFunction::Add);
    return isSuccess;
}

bool ScoreBoard_Reduce(Player& player, int score, string const& scoreName) {
    Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
    Objective*  obj        = scoreboard.getObjective(scoreName);
    if (!obj) {
        sendText<LogLevel::Error>(player, "[Moneys] 插件错误: 找不到指定的计分板: "_tr(scoreName));
        return false;
    }
    const ScoreboardId& id = scoreboard.getScoreboardId(player);
    if (!id.isValid()) {
        scoreboard.createScoreboardId(player);
    }
    bool isSuccess = false;
    scoreboard.modifyPlayerScore(isSuccess, id, *obj, score, PlayerScoreSetFunction::Subtract);
    return isSuccess;
}


// EconomySystem
EconomySystem& EconomySystem::getInstance() {
    static EconomySystem instance;
    return instance;
}

bool EconomySystem::updateConfig(EconomyConfig Config) {
    this->mEconomyConfig = Config;
    return true;
}


long long EconomySystem::get(Player& player) {
    switch (mEconomyConfig.type) {
    case EconomyType::ScoreBoard:
        return ScoreBoard_Get(player, mEconomyConfig.scoreName);
    case EconomyType::LegacyMoney:
        return LLMoney_Get(player.getXuid());
    default:
        return 0;
    }
}


bool EconomySystem::set(Player& player, long long money) {
    switch (mEconomyConfig.type) {
    case EconomyType::ScoreBoard:
        return ScoreBoard_Set(player, money, mEconomyConfig.scoreName);
    case EconomyType::LegacyMoney:
        return LLMoney_Set(player.getXuid(), money);
    default:
        return false;
    }
}


bool EconomySystem::add(Player& player, long long money) {
    if (!mEconomyConfig.enable) return true; // 未启用则不限制
    switch (mEconomyConfig.type) {
    case EconomyType::ScoreBoard:
        return ScoreBoard_Add(player, money, mEconomyConfig.scoreName);
    case EconomyType::LegacyMoney:
        return LLMoney_Add(player.getXuid(), money);
    default:
        return false;
    }
}


bool EconomySystem::reduce(Player& player, long long money) {
    if (!mEconomyConfig.enable) return true; // 未启用则不限制
    if (get(player) >= money) {              // 防止玩家余额不足
        switch (mEconomyConfig.type) {
        case EconomyType::ScoreBoard:
            return ScoreBoard_Reduce(player, money, mEconomyConfig.scoreName);
        case EconomyType::LegacyMoney:
            return LLMoney_Reduce(player.getXuid(), money);
        default:
            return false;
        }
    }
    // 封装提示信息
    sendLackMoneyTip(player, money);
    return false;
}


string EconomySystem::getMoneySpendTipStr(Player& player, long long money) {
    long long currentMoney = mEconomyConfig.enable ? get(player) : 0;
    string    prefix       = "\n[§uTip§r]§r ";

    auto& name = mEconomyConfig.economicName;

    if (mEconomyConfig.enable)
        return prefix
             + "此操作消耗§6{0}§r:§e{1}§r | 当前§6{2}§r:§d{3}§r | 剩余§6{4}§r:§s{5}§r | {6}"_tr(
                   name,
                   money,
                   name,
                   currentMoney,
                   name,
                   currentMoney - money,
                   currentMoney >= money ? "§6{}§r§a充足§r"_tr(name) : "§6{}§r§c不足§r"_tr(name)
             );
    else return prefix + "经济系统未启用，此操作不消耗§6{0}§r"_tr(name);
}

void EconomySystem::sendLackMoneyTip(Player& player, long long money) {
    sendText<LogLevel::Error>(
        player,
        "[Moneys] 操作失败，此操作需要{0}:{1}，当前{2}:{3}"_tr(
            mEconomyConfig.economicName,
            money,
            mEconomyConfig.economicName,
            get(player)
        )
    );
}

} // namespace plo::utils
