#pragma once
#include "Date.h"
#include "Utils.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Global.h"

using string = std::string;

namespace plo::utils {


enum class EconomyType : int { Unknown = -1, LegacyMoney = 0, ScoreBoard = 1 };
struct EconomyConfig {
    bool        enable       = false;
    EconomyType type         = EconomyType::LegacyMoney;
    string      scoreName    = "";
    string      economicName = "金币";
};

class EconomySystem {
public:
    EconomyConfig mEconomyConfig;

    EconomySystem()                                = default;
    EconomySystem(const EconomySystem&)            = delete;
    EconomySystem& operator=(const EconomySystem&) = delete;

    PLAPI static EconomySystem& getInstance();
    PLAPI bool                  updateConfig(EconomyConfig Config);

    PLAPI long long get(Player& player);

    PLAPI bool set(Player& player, long long money);

    PLAPI bool add(Player& player, long long money);

    PLAPI bool reduce(Player& player, long long money);

    PLAPI string getMoneySpendTipStr(Player& player, long long money);

    PLAPI void sendLackMoneyTip(Player& player, long long money); // 发送经济不足提示
};


} // namespace plo::utils
