#include "Global.h"


namespace plo::gui {


void PlotShopGUI(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 地皮商店(玩家出售)");

    auto* impl = &data::PlotDBStorage::getInstance();

    auto sls = impl->getSaleingPlots();
    for (auto const& sl : sls) {
        auto pt = impl->getPlot(sl->getPlotID());
        if (!pt) continue;
        if (pt->isOwner(player.getUuid().asString())) continue; // 跳过自己的地皮
        fm.appendButton(
            fmt::format("{}\nID: {} | 价格: {}", pt->getPlotName(), sl->getPlotID(), sl->getSalePrice()),
            [pt, sl](Player& pl) { _plotShopShowPlot(pl, pt); }
        );
    }

    fm.sendTo(player);
}

void _plotShopShowPlot(Player& player, PlotMetadataPtr pt) {
    SimpleForm fm{PLUGIN_TITLE};

    auto* ndb = &PlayerNameDB::getInstance();

    fm.setContent(fmt::format(
        "地皮ID: {}\n地皮名称: {}\n地皮主人: {}\n出售价格: {}",
        pt->getPlotID(),
        pt->getPlotName(),
        ndb->getPlayerName(pt->getPlotOwner()),
        pt->getSalePrice()
    ));

    fm.appendButton("购买地皮", "textures/ui/confirm", "path", [pt](Player& pl) { _buyPlot(pl, pt); });

    fm.appendButton("传送到此地皮", "textures/ui/send_icon", "path", [pt](Player& pl) {
        PlotPos pps{pt->getX(), pt->getZ()};
        pl.teleport(pps.getSafestPos(), getPlotWorldDimensionId());
    });

    fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { PlotShopGUI(pl); });

    fm.sendTo(player);
}


void _buyPlot(Player& player, PlotMetadataPtr pt) {

    auto* impl = &data::PlotDBStorage::getInstance();
    auto& cfg  = Config::cfg.plotWorld;

    if (static_cast<int>(impl->getPlots(player.getUuid().asString()).size()) >= cfg.maxBuyPlotCount) {
        sendText<LogLevel::Warn>(player, "你已经购买了太多地皮，无法购买新的地皮。");
        return;
    }

    auto*      ms       = &EconomySystem::getInstance();
    bool const hasOwner = !pt->getPlotOwner().empty();
    bool const hasSale  = pt->isSale();
    bool const fromSale = hasOwner && hasSale; // 是否从出售地皮购买(玩家)
    int const  price    = fromSale ? pt->getSalePrice() : cfg.buyPlotPrice;

    if (hasOwner && !hasSale) {
        sendText<LogLevel::Warn>(player, "这个地皮没有出售，无法购买。");
        return;
    }
    if (pt->isOwner(player.getUuid().asString())) {
        sendText<LogLevel::Warn>(player, "你不能购买自己的地皮。");
        return;
    }

    pev::PlayerBuyPlotBefore ev{&player, pt, price};
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    ModalForm{
        PLUGIN_TITLE,
        fmt::format("是否确认购买地皮 {} ?\n{}", pt->getPlotID(), ms->getMoneySpendTipStr(player, price)),
        "确认",
        "返回"
    }
        .sendTo(player, [pt, price, fromSale, ms, impl, cfg](Player& pl, ModalFormResult const& dt, FormCancelReason) {
            if (!dt) {
                sendText(pl, "表单已放弃");
                return;
            }
            if (!(bool)dt.value()) {
                PlotGUI(pl, pt, true);
                return;
            }

            if (ms->reduce(pl, price)) {
                pev::PlayerBuyPlotAfter ev{&pl, pt, price};
                auto&                   bus = ll::event::EventBus::getInstance();
                if (fromSale) {
                    bool const ok = impl->buyPlotFromSale(pt->getPlotID(), pl.getUuid().asString());
                    if (ok) {
                        // 计算税率
                        int const tax      = cfg.playerSellPlotTax * price / 100;
                        int       newPrice = price - tax;
                        if (newPrice < 0) newPrice = 0;
                        // 把扣除的经济转移给出售者
                        auto plptr = ll::service::getLevel()->getPlayer(pt->getPlotOwner());
                        if (plptr) {
                            ms->add(*plptr, newPrice); // 出售者(当前地皮主人)在线
                            sendText(
                                plptr,
                                "玩家 {} 购买了您出售的地皮 {}, 经济 +{}",
                                pl.getRealName(),
                                pt->getPlotName(),
                                newPrice
                            );
                        } else {
                            EconomyQueue::getInstance().set(pt->getPlotOwner(), newPrice); // 出售者(当前地皮主人)离线
                        }

                        bus.publish(ev);
                        sendText(pl, "地皮购买成功");
                    } else sendText<LogLevel::Error>(pl, "地皮购买失败");
                } else {
                    PlotPos    ps{pt->getX(), pt->getZ()};
                    bool const ok = impl->addPlot(ps.getPlotID(), pl.getUuid().asString(), pt->getX(), pt->getZ());
                    if (ok) {
                        bus.publish(ev);
                        sendText(pl, "地皮购买成功");
                    } else sendText<LogLevel::Error>(pl, "地皮购买失败");
                }
            }
        });
}


} // namespace plo::gui