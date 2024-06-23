#include "index.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/utils/Moneys.h"
#include <cstdint>
#include <regex>

using namespace ll::form;
using namespace plo::utils;
namespace pev = plo::event;

namespace plo::gui {

void index(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 选择一个操作:");

    if (player.getDimensionId() == VanillaDimensions::fromString("plot")) {
        fm.appendButton("前往主世界", [](Player& pl) { mc::executeCommand("plo go overworld", &pl); });
    } else {
        fm.appendButton("前往地皮世界", [](Player& pl) { mc::executeCommand("plo go plot", &pl); });
    }

    fm.appendButton("管理脚下地皮", [](Player& pl) { mc::executeCommand("plo plot", &pl); });

    fm.appendButton("管理地皮", [](Player& pl) { _selectPlot(pl); });

    // fm.appendButton("插件设置", [](Player& pl) {});

    fm.appendButton("退出", [](Player&) {});
    fm.sendTo(player);
}


void _selectPlot(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.appendButton("返回", [](Player& pl) { index(pl); });


    auto plots = PlotDB::getInstance().getCachedPlots();

    for (auto const& plt : plots) {
        if (plt.mPlotOwner != player.getUuid()) continue; // 不是自己的地皮
        fm.appendButton(fmt::format("{}\n{}", plt.mPlotName, plt.mPlotID), [plt](Player& pl) { plot(pl, plt, true); });
    }

    fm.sendTo(player);
}


// command index
void plot(Player& player, PlotPos plotPos) {
    auto& db = PlotDB::getInstance();

    auto pt = db.getCached(plotPos.getPlotID());

    if (pt.has_value()) {
        plot(player, pt.value());
    } else {
        plot(player, Plot{plotPos.getPlotID(), "", UUID{}, plotPos.x, plotPos.z}); // fake plot
    }
}


void plot(Player& player, Plot pt, bool ret) {
    SimpleForm fm{PLUGIN_TITLE};

    auto& ndb  = PlayerNameDB::getInstance();
    auto& pdb  = PlotDB::getInstance();
    auto& impl = pdb.getImpl();

    bool const hasOwner = !pt.mPlotOwner.isEmpty();                      // 是否有主人
    bool const hasSale  = impl.hasSale(pt.mPlotID);                      // 是否出售
    bool const isOwner  = hasOwner && player.getUuid() == pt.mPlotOwner; // 是否是主人

    fm.setContent(fmt::format(
        "地皮 {} 的元数据:\n地皮主人: {}\n地皮名称: {}\n是否出售: {}\n出售价格: {}\n  ",
        pt.mPlotID,
        hasOwner ? "无主" : ndb.getPlayerName(pt.mPlotOwner),
        pt.mPlotName,
        hasOwner ? hasSale ? "是" : "否" : "否",
        hasOwner ? hasSale ? std::to_string(impl.getSale(pt.mPlotID)->mPrice) : "null"
                 : std::to_string(config::cfg.plotWorld.buyPlotPrice)
    ));

    if (ret) fm.appendButton("返回", [](Player& pl) { _selectPlot(pl); });

    if ((!hasOwner || hasSale) && !isOwner) fm.appendButton("购买地皮", [pt](Player& pl) { _buyPlot(pl, pt); });

    if (isOwner) {
        fm.appendButton("修改地皮名称", [pt](Player& pl) { _changePlotName(pl, pt); });

        fm.appendButton("地皮出售", [pt](Player& pl) { _sellMyPlot(pl, pt); });

        fm.appendButton("共享地皮", [pt](Player& pl) { _plotShareManage(pl, pt); });
    }

    if (hasOwner) fm.appendButton("地皮评论", [pt](Player& pl) { _plotcomment(pl, pt); });


    fm.sendTo(player);
}


void _sellMyPlot(Player& player, Plot pt) {
    auto* impl = &PlotDB::getInstance().getImpl();

    bool const isSaleing = impl->hasSale(pt.mPlotID);

    SimpleForm fm{PLUGIN_TITLE};

    if (isSaleing) {
        fm.setContent(fmt::format("你正在出售地皮 {}，价格为 {}。", pt.mPlotID, impl->getSale(pt.mPlotID)->mPrice));

        fm.appendButton("编辑出售价格", [pt](Player& pl) { _sellPlotAndEditPrice(pl, pt, true); });
        fm.appendButton("取消出售", [pt, impl](Player& pl) {
            bool const ok = impl->removeSale(pt.mPlotID);
            if (ok) sendText(pl, "出售已取消");
            else sendText<utils::Level::Error>(pl, "出售取消失败");
        });
    } else {
        fm.setContent("此地皮没有出售，无法查询信息。");

        fm.appendButton("出售当前地皮", [pt](Player& pl) { _sellPlotAndEditPrice(pl, pt, false); });
    }

    fm.appendButton("返回", [pt](Player& pl) { plot(pl, pt, true); });

    fm.sendTo(player);
}


void _sellPlotAndEditPrice(Player& player, Plot pt, bool edit) {
    auto* impl = &PlotDB::getInstance().getImpl();

    CustomForm fm{PLUGIN_TITLE};

    fm.appendLabel("地皮确认出售后，其它玩家可以查看到你的出售信息。当玩家购买后，你的地皮将被转移到购买者的名下("
                   "自动重置共享信息)。");

    if (edit) fm.appendInput("pr", "请输入出售价格:", "integer", std::to_string(impl->getSale(pt.mPlotID)->mPrice));
    else fm.appendInput("pr", "请输入出售价格:", "integer");

    fm.sendTo(player, [pt, impl, edit](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        string const pr = std::get<string>(dt->at("pr")); // 输入框只能输入字符串

        if (!std::regex_match(pr, std::regex("^\\d+$"))) {
            sendText<utils::Level::Error>(pl, "价格必须为整数");
            return;
        }

        int const p = std::stoi(pr);
        if (p <= 0) {
            sendText<utils::Level::Error>(pl, "价格必须大于0");
            return;
        }

        if (edit) {
            bool const ok = impl->updateSale(pt.mPlotID, p);
            if (ok) sendText(pl, "出售价格已修改");
            else sendText<utils::Level::Error>(pl, "出售价格修改失败");
        } else {
            bool const ok = impl->addSale(pt.mPlotID, p);
            if (ok) sendText(pl, "出售成功");
            else sendText<utils::Level::Error>(pl, "出售失败");
        }
    });
}


void _plotShareManage(Player& player, Plot pt) {
    auto* impl = &PlotDB::getInstance().getImpl();
    auto* ndb  = &PlayerNameDB::getInstance();


    SimpleForm fm{PLUGIN_TITLE};
    fm.setContent("地皮共享设置\n将玩家设置为当前地皮共享者(信任者)后\n被授权的玩家拥有共享的地皮权限(放置、破坏、修改)"
                  "。注意：此权限不包含打开GUI修改地皮的权限。");

    fm.appendButton("返回", [pt](Player& pl) { plot(pl, pt, true); });

    fm.appendButton("清除所有共享者", [pt, impl](Player& pl) {
        bool const ok = impl->resetPlotShareInfo(pt.mPlotID);
        if (ok) sendText(pl, "共享信息已清除");
        else sendText<utils::Level::Error>(pl, "共享信息清除失败");
    });

    fm.appendButton("添加共享者", [pt](Player& pl) { _addSharePlayer(pl, pt); });


    auto sharedInfos = impl->getSharedPlots(pt.mPlotID);
    for (auto const& si : sharedInfos) {
        fm.appendButton(
            fmt::format("{}\n{}", ndb->getPlayerName(si.mSharedPlayer), si.mSharedTime),
            [pt, si, ndb, impl](Player& pl) {
                ModalForm{
                    PLUGIN_TITLE,
                    fmt::format(
                        "地皮ID: {}\n共享者: {}\n共享时间: {}",
                        si.mPlotID,
                        ndb->getPlayerName(si.mSharedPlayer),
                        si.mSharedTime
                    ),
                    "删除此玩家的共享权限",
                    "返回"
                }
                    .sendTo(pl, [pt, si, ndb, impl](Player& pl, ModalFormResult const& dt, FormCancelReason) {
                        if (!dt) {
                            sendText(pl, "表单已放弃");
                            return;
                        }
                        if (!(bool)dt.value()) {
                            plot(pl, pt, true);
                            return;
                        }

                        bool const ok = impl->removeSharedInfo(si.mPlotID, si.mSharedPlayer);
                        if (ok) _plotShareManage(pl, pt);
                        else
                            sendText<utils::Level::Error>(
                                pl,
                                "删除玩家 {} 的共享权限失败",
                                ndb->getPlayerName(si.mSharedPlayer)
                            );
                    });
            }
        );
    }

    fm.sendTo(player);
}

void _addSharePlayer(Player& player, Plot pt) {
    auto* impl = &PlotDB::getInstance().getImpl();
    auto* ndb  = &PlayerNameDB::getInstance();

    CustomForm fm{PLUGIN_TITLE};

    std::vector<string> names;
    ll::service::getLevel()->forEachPlayer([&names, &player](Player& p) {
        if (p == player) return true; // 排除自己
        names.push_back(p.getRealName());
        return true;
    });

    fm.appendDropdown("sp", "[在线] 选择共享者:", names);

    fm.appendInput("in", "[离线] 输入共享者的名字:", "string");

    fm.appendToggle("sw", "在线 <-> 离线");

    fm.sendTo(player, [pt, impl, ndb](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        bool const   sw = std::get<uint64_t>(dt->at("sw"));
        string const sp = std::get<string>(dt->at("sp"));
        string const in = std::get<string>(dt->at("in"));

        bool ok = false;

        if (sw) {
            // 离线
            if (in.empty()) {
                sendText(pl, "输入共享者的名字不能为空");
                return;
            }

            auto const uuid = ndb->getPlayerUUID(in);
            if (!uuid.has_value()) {
                sendText(pl, "输入的共享者不存在,获取UUID失败");
                return;
            }

            ok = impl->addShareInfo(pt.mPlotID, uuid.value());
        } else {
            // 在线
            ok = impl->addShareInfo(pt.mPlotID, *ndb->getPlayerUUID(sp));
        }

        if (ok) sendText(pl, "共享权限添加成功");
        else sendText<utils::Level::Error>(pl, "共享权限添加失败");
    });
}


void _changePlotName(Player& player, Plot pt) {
    auto* bus = &ll::event::EventBus::getInstance();

    pev::PlayerChangePlotNameBefore ev{&player, pt};
    bus->publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("pn", "地皮名称:", "string", pt.mPlotName);

    fm.sendTo(player, [pt, bus](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        string pn = std::get<string>(dt->at("pn"));

        if (pn.empty()) {
            sendText(pl, "地皮名称不能为空");
            return;
        }

        auto&      impl = PlotDB::getInstance().getImpl();
        bool const ok   = impl.updatePlotName(pt.mPlotID, pn);

        if (ok) {
            pev::PlayerChangePlotNameAfter ev{&pl, pt, pn};
            bus->publish(ev);

            sendText(pl, "地皮名称已修改");
        } else {
            sendText<utils::Level::Error>(pl, "地皮名称修改失败");
        }
    });
}


void _buyPlot(Player& player, Plot pt) {

    auto* impl = &PlotDB::getInstance().getImpl();
    auto& cfg  = config::cfg.plotWorld;

    if (static_cast<int>(impl->getPlots(player.getUuid()).size()) >= cfg.maxBuyPlotCount) {
        sendText<utils::Level::Warn>(player, "你已经购买了太多地皮，无法购买新的地皮。");
        return;
    }


    auto*      ms       = &Moneys::getInstance();
    bool const hasOwner = !pt.mPlotOwner.isEmpty();
    bool const hasSale  = impl->hasSale(pt.mPlotID);
    bool const fromSale = hasOwner && hasSale; // 是否从出售地皮购买(玩家)
    int const  price    = fromSale ? impl->getSale(pt.mPlotID)->mPrice : cfg.buyPlotPrice;

    if (hasOwner && !hasSale) {
        sendText<utils::Level::Warn>(player, "这个地皮没有出售，无法购买。");
        return;
    }

    pev::PlayerBuyPlotBefore ev{&player, pt, price};
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    ModalForm{
        PLUGIN_TITLE,
        fmt::format("是否确认购买地皮 {} ?\n{}", pt.mPlotID, ms->getMoneySpendTipStr(player, price)),
        "确认",
        "返回"
    }
        .sendTo(player, [pt, price, fromSale, ms, impl](Player& pl, ModalFormResult const& dt, FormCancelReason) {
            if (!dt) {
                sendText(pl, "表单已放弃");
                return;
            }
            if (!(bool)dt.value()) {
                plot(pl, pt, true);
                return;
            }

            if (ms->reduceMoney(pl, price)) {
                pev::PlayerBuyPlotAfter ev{&pl, pt, price};
                auto&                   bus = ll::event::EventBus::getInstance();
                if (fromSale) {
                    bool const ok = impl->buyPlotFromSale(pt.mPlotID, pl.getUuid());
                    if (ok) {
                        bus.publish(ev);
                        sendText(pl, "地皮购买成功");
                    } else sendText<utils::Level::Error>(pl, "地皮购买失败");
                } else {
                    PlotPos    ps{pt.mPlotX, pt.mPlotZ};
                    bool const ok = impl->addPlot(ps, pl.getUuid(), pt.mPlotID);
                    if (ok) {
                        bus.publish(ev);
                        sendText(pl, "地皮购买成功");
                    } else sendText<utils::Level::Error>(pl, "地皮购买失败");
                }
            }
        });
}


void _plotcomment(Player& player, Plot pt) {

    bool const hasOwner = !pt.mPlotOwner.isEmpty(); // 是否有主人

    if (!hasOwner) {
        sendText<utils::Level::Warn>(player, "你不能评论这个地皮，因为它没有主人。");
        return;
    }

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent(fmt::format("地皮 '{}' 的评论区:", pt.mPlotID));

    fm.appendButton("返回", [pt](Player& pl) { plot(pl, pt, true); });

    fm.appendButton("发表评论", [pt](Player& pl) { _publishComment(pl, pt); });


    auto& ndb = PlayerNameDB::getInstance();
    auto  cts = PlotDB::getInstance().getImpl().getComments(pt.mPlotID);

    std::sort(cts.begin(), cts.end(), [](auto const& a, auto const& b) {
        return Date::parse(a.mCommentTime) < Date::parse(b.mCommentTime);
    });

    for (auto const& ct : cts) {
        fm.appendButton(
            fmt::format("{}  {}\n{}", ndb.getPlayerName(ct.mCommentPlayer), ct.mCommentTime, ct.mContent),
            [ct, pt](Player& pl) { _showCommentOperation(pl, pt, ct); }
        );
    }

    fm.sendTo(player);
}


void _publishComment(Player& player, Plot pt) {
    pev::PlayerCommentPlotBefore ev{&player, pt};
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("cm", "评论内容:", "string");

    fm.sendTo(player, [pt](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        string ct = std::get<string>(dt->at("cm"));

        if (ct.empty()) {
            sendText(pl, "评论内容不能为空");
            return;
        }

        auto&      impl = PlotDB::getInstance().getImpl();
        bool const ok   = impl.addComment(pt.mPlotID, pl.getUuid(), ct);

        if (ok) {
            pev::PlayerCommentPlotAfter ev{&pl, pt, ct};
            ll::event::EventBus::getInstance().publish(ev);

            sendText(pl, "评论已发布");
        } else {
            sendText<utils::Level::Error>(pl, "评论发布失败");
        }
    });
}


void _showCommentOperation(Player& player, Plot pt, PlotComment ct) {

    bool const isOwner        = player.getUuid() == pt.mPlotOwner;
    bool const isCommentOwner = player.getUuid() == ct.mCommentPlayer;

    auto& ndb = PlayerNameDB::getInstance();

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent(fmt::format(
        "评论ID: {}\n评论时间: {}\n评论玩家: {}\n评论内容: {}",
        ct.mCommentID,
        ct.mCommentTime,
        ndb.getPlayerName(ct.mCommentPlayer),
        ct.mContent
    ));

    if (isCommentOwner) {
        fm.appendButton("编辑评论", [pt, ct](Player& pl) { _editComment(pl, pt, ct); });
    }

    if (isCommentOwner || isOwner) {
        fm.appendButton("删除评论", [pt, ct](Player& pl) {
            pev::PlayerDeletePlotComment ev{&pl, pt, ct};
            ll::event::EventBus::getInstance().publish(ev);
            if (ev.isCancelled()) return; // 事件被取消

            auto&      impl = PlotDB::getInstance().getImpl();
            bool const ok   = impl.removeComment(ct.mCommentID);

            if (ok) {
                sendText(pl, "评论已删除");
            } else {
                sendText<utils::Level::Error>(pl, "评论删除失败");
            }
        });
    }

    fm.appendButton("返回", [pt](Player& pl) { _plotcomment(pl, pt); });

    fm.sendTo(player);
}


void _editComment(Player& player, Plot pt, PlotComment ct) {
    pev::PlayerEditCommentBefore ev{&player, pt, ct};
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("cm", "评论内容:", "string", ct.mContent);

    fm.sendTo(player, [pt, ct](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        string cm = std::get<string>(dt->at("cm"));

        if (cm.empty()) {
            sendText(pl, "评论内容不能为空");
            return;
        }

        auto&      impl = PlotDB::getInstance().getImpl();
        bool const ok   = impl.updateComment(ct.mCommentID, cm);

        if (ok) {
            pev::PlayerEditCommentAfter ev{&pl, pt, ct, cm};
            ll::event::EventBus::getInstance().publish(ev);

            sendText(pl, "评论已修改");
        } else {
            sendText<utils::Level::Error>(pl, "评论修改失败");
        }
    });
}


} // namespace plo::gui