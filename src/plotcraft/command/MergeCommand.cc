#include "Command.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomySystem.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/math/PlotCross.h"
#include "plotcraft/math/PlotPos.h"
#include "plotcraft/math/PlotRoad.h"
#include "plotcraft/utils/Mc.h"
#include <memory>
#include <unordered_map>
#include <utility>



namespace plot::command {


class MergeBindData {
public:
    MergeBindData()                                = delete;
    MergeBindData(MergeBindData const&)            = delete;
    MergeBindData& operator=(MergeBindData const&) = delete;

    /**
     * @note 玩家数据绑定类
     * @note 用于记录每个玩家的合并操作数据
     * @note 使用 uuid 作为 key
     * @note 使用 std::pair<PlotPos, PlotPos> 作为 value
     * @note value.first 为源地皮，value.second 为目标地皮
     * @note value.first 和 value.second 必须是相邻地皮
     */
    static std::unordered_map<UUIDm, std::pair<PlotPos, PlotPos>> mBindData;

    static bool isEnabled(Player const& player) { return mBindData.find(player.getUuid()) != mBindData.end(); }

    static bool enable(Player& player) {
        if (isEnabled(player)) {
            return false;
        }
        mBindData[player.getUuid()] = std::make_pair(PlotPos(), PlotPos());
        return true;
    }

    static bool disable(Player& player) {
        mBindData.erase(player.getUuid());
        return true;
    }

    static std::pair<PlotPos, PlotPos>* getBindData(Player const& player) {
        if (!isEnabled(player)) {
            return nullptr;
        }
        return &mBindData[player.getUuid()];
    }

    static bool setSource(Player& player, PlotPos const& pos) {
        if (!isEnabled(player)) {
            return false;
        }
        mBindData[player.getUuid()].first = pos;
        return true;
    }

    static bool setTarget(Player& player, PlotPos const& pos) {
        if (!isEnabled(player)) {
            return false;
        }
        mBindData[player.getUuid()].second = pos;
        return true;
    }
};
std::unordered_map<UUIDm, std::pair<PlotPos, PlotPos>> MergeBindData::mBindData;

enum class SetType : int {
    Source,
    Target,
};
struct SetParam {
    SetType type;
};


void _SetUpMergeCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(COMMAND_NAME, "PlotCraft");

    // plot merge enable
    cmd.overload().text("merge").text("enable").execute([](CommandOrigin const& ori, CommandOutput& out) {
        Actor* _ent = ori.getEntity();
        if (!_ent || !_ent->isPlayer()) {
            mc::sendText<mc::LogLevel::Error>(out, "此命令仅限玩家使用");
            return;
        }
        Player* player = static_cast<Player*>(_ent);

        if (MergeBindData::isEnabled(*player)) {
            mc::sendText<mc::LogLevel::Error>(out, "您已开启合并功能，请先完成操作或取消操作");
            return;
        }

        if (MergeBindData::enable(*player)) {
            mc::sendText(out, "已开启合并功能，请选择源地皮和目标地皮");
        } else {
            mc::sendText<mc::LogLevel::Error>(out, "开启合并功能失败，请重试");
        }
    });

    // plot merge disable
    cmd.overload().text("merge").text("disable").execute([](CommandOrigin const& ori, CommandOutput& out) {
        Actor* _ent = ori.getEntity();
        if (!_ent || !_ent->isPlayer()) {
            mc::sendText<mc::LogLevel::Error>(out, "此命令仅限玩家使用");
            return;
        }
        Player* player = static_cast<Player*>(_ent);

        if (!MergeBindData::isEnabled(*player)) {
            mc::sendText<mc::LogLevel::Error>(out, "您未开启合并功能，没有什么可关闭的");
            return;
        }

        if (MergeBindData::disable(*player)) {
            mc::sendText(out, "合并功能已关闭");
        } else {
            mc::sendText<mc::LogLevel::Error>(out, "关闭合并功能失败，请重试");
        }
    });

    // plot merge set <source|target>
    cmd.overload<SetParam>().text("merge").text("set").required("type").execute(
        [](CommandOrigin const& ori, CommandOutput& out, SetParam const& param) {
            Actor* _ent = ori.getEntity();
            if (!_ent || !_ent->isPlayer()) {
                mc::sendText<mc::LogLevel::Error>(out, "此命令仅限玩家使用");
                return;
            }
            Player* player = static_cast<Player*>(_ent);

            if (!MergeBindData::isEnabled(*player)) {
                mc::sendText<mc::LogLevel::Error>(out, "您未开启合并功能，请先开启合并功能");
                return;
            }

            PlotPos pos = PlotPos(player->getPosition());
            if (!pos.isValid()) {
                mc::sendText<mc::LogLevel::Error>(out, "当前位置不在地皮内，请选择地皮内位置");
                return;
            }

            auto& db   = data::PlotDBStorage::getInstance();
            auto  plot = db.getPlot(pos.getPlotID());
            if (!plot) {
                mc::sendText<mc::LogLevel::Error>(out, "当前地皮没有主人，您无法选择此地皮");
                return;
            }

            if (!plot->isOwner(player->getUuid().asString())) {
                mc::sendText<mc::LogLevel::Error>(out, "您不是当前地皮的主人，无法选择此地皮");
                return;
            }

            switch (param.type) {
            case SetType::Source: {
                if (MergeBindData::setSource(*player, pos)) {
                    mc::sendText(out, "已设置 {} 为源地皮", pos.getPlotID());
                } else {
                    mc::sendText<mc::LogLevel::Error>(out, "设置源地皮失败，请重试");
                }
                break;
            }
            case SetType::Target: {
                if (MergeBindData::setTarget(*player, pos)) {
                    mc::sendText(out, "已设置 {} 为目标地皮", pos.getPlotID());
                } else {
                    mc::sendText<mc::LogLevel::Error>(out, "设置目标地皮失败，请重试");
                }
                break;
            }
            }
        }
    );

    // plot merge confirm
    cmd.overload().text("merge").text("confirm").execute([](CommandOrigin const& ori, CommandOutput& out) {
        Actor* _ent = ori.getEntity();
        if (!_ent || !_ent->isPlayer()) {
            mc::sendText<mc::LogLevel::Error>(out, "此命令仅限玩家使用");
            return;
        }
        Player* player = static_cast<Player*>(_ent);

        if (!MergeBindData::isEnabled(*player)) {
            mc::sendText<mc::LogLevel::Error>(out, "您未开启合并功能，请先开启合并功能");
            return;
        }

        auto _bindData = MergeBindData::getBindData(*player);
        if (!_bindData) {
            mc::sendText<mc::LogLevel::Error>(out, "您未选择任何地皮，请先选择地皮");
            return;
        }

        auto const& souPos = _bindData->first;
        auto const& tarPos = _bindData->second;

        if (!souPos.isValid() || !tarPos.isValid()) {
            mc::sendText(out, "源地皮或目标地皮无效，请重新选择地皮");
            return;
        }

        if (souPos == tarPos) {
            mc::sendText(out, "源地皮和目标地皮相同，请重新选择地皮");
            return;
        }

        if (!PlotPos::isAdjacent(souPos, tarPos)) {
            mc::sendText(out, "源地皮和目标地皮不是相邻地皮，请重新选择地皮");
            return;
        }

        auto& db = data::PlotDBStorage::getInstance();

        auto souPlot = db.getPlot(souPos.getPlotID());
        auto tarPlot = db.getPlot(tarPos.getPlotID());
        if (!souPlot || !tarPlot) {
            mc::sendText(out, "当前地皮不存在，请重新选择地皮");
            return;
        }

        int count = souPlot->mMergedData.mMergeCount + tarPlot->mMergedData.mMergeCount + 1;
        if (count > Config::cfg.plotWorld.maxMergePlotCount) {
            mc::sendText<mc::LogLevel::Error>(out, "合并次数超过限制，无法合并");
            return;
        }

        int   price = (int)Config::calculateMergePlotPrice(count);
        auto& eco   = EconomySystem::getInstance();
        if (!eco.reduce(*player, price)) {
            eco.sendNotEnoughMessage(*player, price);
            mc::sendText<mc::LogLevel::Error>(out, "您的余额不足，无法合并");
            return;
        }

        mc::sendText(out, "正在处理地皮...");
        auto resultPtr = souPos.tryMerge(tarPos);
        if (!resultPtr) {
            mc::sendText<mc::LogLevel::Error>(out, "合并失败，请重试");
            return;
        }
        // fmt::print("resultPtr 地址: {}\n", fmt::ptr(std::addressof(*resultPtr)));

        Block const& block = Block::tryGetFromRegistry(Config::cfg.generator.fillBlock);
        auto         roads = std::addressof(*resultPtr)->getRangedRoads();
        for (auto& i : roads) {
            if (i.mIsMergedPlot) continue; // 已经被合并，不再填充
            i.fill(block, true);
        }
        auto crosses = std::addressof(*resultPtr)->getRangedCrosses();
        for (auto& i : crosses) {
            if (i.mIsMergedPlot) continue; // 已经被合并，不再填充
            i.fill(block, true);
        }

        souPlot->updateMergeData(resultPtr);
        souPlot->setMergeCount(count);
        souPlot->mergeData(tarPlot);
        db._archivePlotData(tarPlot->getPlotID());
        db.refreshMergeMap();
        resultPtr->fixBorder();
        MergeBindData::disable(*player);
        mc::sendText(out, "地皮合并完成");
    });
}


} // namespace plot::command
