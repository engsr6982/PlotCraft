#pragma once
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Global.h"


namespace plot {


class EconomySystem {
public:
    enum class EconomyKit { LegacyMoney, Scoreboard };
    struct Config {
        bool       enabled{false};               // 是否启用经济系统
        EconomyKit kit{EconomyKit::LegacyMoney}; // 经济系统类型
        string     currency{"money"};            // 货币名称
        string     scoreboard{""};               // Scoreboard 经济系统使用的计分板名称
    };

    Config const* mConfig; // 经济系统配置

    EconomySystem()                                = default;
    EconomySystem(const EconomySystem&)            = delete;
    EconomySystem& operator=(const EconomySystem&) = delete;

    PLAPI static EconomySystem& getInstance();
    PLAPI void                  update(Config const* config);

    /**
     * @brief 是否装载经济前置LegacyMoney
     */
    PLAPI bool isLegacyMoneyLoaded() const;

    /**
     * @brief 获取玩家余额
     */
    PLAPI llong get(Player& player) const;        // 在线API
    PLAPI llong get(mce::UUID const& uuid) const; // 离线API

    /**
     * @brief 设置玩家余额
     */
    PLAPI bool set(Player& player, llong amount) const;
    PLAPI bool set(mce::UUID const& uuid, llong amount) const;

    /**
     * @brief 增加玩家余额
     */
    PLAPI bool add(Player& player, llong amount) const;
    PLAPI bool add(mce::UUID const& uuid, llong amount) const;

    /**
     * @brief 减少玩家余额
     */
    PLAPI bool reduce(Player& player, llong amount) const;
    PLAPI bool reduce(mce::UUID const& uuid, llong amount) const;

    /**
     * @brief 转移玩家余额
     */
    PLAPI bool transfer(Player& player, Player& target, llong amount) const;
    PLAPI bool transfer(mce::UUID const& uuid, mce::UUID const& target, llong amount) const;

    /**
     * @brief 生成经济花费提示模板
     */
    PLAPI string getCostMessage(Player& player, llong cost) const;

    /**
     * @brief 发送经济不足提示
     */
    PLAPI void sendNotEnoughMessage(Player& player, llong cost) const;

    PLAPI static void sendErrorMessage(Player& player, string const& message);
    PLAPI static void sendMessage(Player& player, string const& message, string const& prefix = "[EconomySystem] §b");
};


} // namespace plot
