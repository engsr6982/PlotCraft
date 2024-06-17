#pragma once
#include "Date.h"
#include "Utils.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"

using string = std::string;

namespace plo::utils {

enum class MoneysType : int { Unknown = -1, LLMoney = 0, ScoreBoard = 1 };

struct MoneysConfig {
    bool       Enable    = false;
    MoneysType MoneyType = MoneysType::LLMoney;
    string     MoneyName = "money";
    string     ScoreName = "";
};

class ScoreBoardMoney {
private:
    string mScoreName;

public:
    PLAPI ScoreBoardMoney(const string& scoreName = "");

    PLAPI void setScoreName(const string& scoreName);

    PLAPI int getScore(Player* player);
    PLAPI int getScore(Player& player);
    PLAPI int getScore(mce::UUID uuid);
    PLAPI int getScore(const string& realName);

    PLAPI bool setScore(Player* player, int score);
    PLAPI bool setScore(Player& player, int score);
    PLAPI bool setScore(mce::UUID uuid, int score);
    PLAPI bool setScore(const string& realName, int score);

    PLAPI bool addScore(Player* player, int score);
    PLAPI bool addScore(Player& player, int score);
    PLAPI bool addScore(mce::UUID uuid, int score);
    PLAPI bool addScore(const string& realName, int score);

    PLAPI bool reduceScore(Player* player, int score);
    PLAPI bool reduceScore(Player& player, int score);
    PLAPI bool reduceScore(mce::UUID uuid, int score);
    PLAPI bool reduceScore(const string& realName, int score);
};


// 继承ScoreBoardMoney
class Moneys : public ScoreBoardMoney {
private:
    bool       mIsEnable = false;
    MoneysType mType     = MoneysType::Unknown; // 经济类型
    string     mMoneyName;                      // 货币名称

    Moneys()                         = default;
    Moneys(const Moneys&)            = delete;
    Moneys& operator=(const Moneys&) = delete;

    PLAPI void throwUnknownType(Player* player = nullptr);

public:
    PLAPI static Moneys& getInstance();
    // 单例模式，服务器启动后请调用此方法初始化
    PLAPI bool updateConfig(MoneysConfig config);

    PLAPI long long getMoney(Player& player);
    PLAPI long long getMoney(Player* player);
    PLAPI long long getMoney(mce::UUID uuid);
    PLAPI long long getMoney(const string& realName);

    PLAPI bool setMoney(Player& player, long long money);
    PLAPI bool setMoney(Player* player, long long money);
    PLAPI bool setMoney(mce::UUID uuid, long long money);
    PLAPI bool setMoney(const string& realName, long long money);

    PLAPI bool addMoney(Player& player, long long money);
    PLAPI bool addMoney(Player* player, long long money);
    PLAPI bool addMoney(mce::UUID uuid, long long money);
    PLAPI bool addMoney(const string& realName, long long money);

    // ! 此 API 已封装经济不足提示，无需手动发送
    PLAPI bool reduceMoney(Player& player, long long money);
    PLAPI bool reduceMoney(Player* player, long long money);
    PLAPI bool reduceMoney(mce::UUID uuid, long long money);
    PLAPI bool reduceMoney(const string& realName, long long money);

    PLAPI string getMoneySpendTipStr(Player& player, long long money);
    PLAPI string getMoneySpendTipStr(Player* player, long long money);
    PLAPI string getMoneySpendTipStr(mce::UUID uuid, long long money);
    PLAPI string getMoneySpendTipStr(const string& realName, long long money);

    PLAPI void sendMoneySpendTip(Player& player, long long money); // 发送经济不足提示
};

} // namespace plo::utils
