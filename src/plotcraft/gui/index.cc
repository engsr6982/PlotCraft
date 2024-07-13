#include "index.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/core/CoreUtils.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plotcraft/utils/Moneys.h"
#include <cstdint>
#include <memory>
#include <regex>


using namespace ll::form;
using namespace plo::utils;
using namespace plo::core_utils;
namespace pev = plo::event;

namespace plo::gui {

void index(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 选择一个操作:");

    if (player.getDimensionId() == getPlotDimensionId()) {
        fm.appendButton("前往主世界", "textures/ui/realmsIcon", "path", [](Player& pl) {
            mc::executeCommand("plo go overworld", &pl);
        });
    } else {
        fm.appendButton("前往地皮世界", "textures/ui/realmsIcon", "path", [](Player& pl) {
            mc::executeCommand("plo go plot", &pl);
        });
    }

    fm.appendButton("管理脚下地皮", "textures/ui/icon_recipe_item", "path", [](Player& pl) {
        mc::executeCommand("plo plot", &pl);
    });

    fm.appendButton("管理地皮", "textures/ui/icon_recipe_nature", "path", [](Player& pl) { _selectPlot(pl); });

    fm.appendButton("地皮商店", "textures/ui/store_home_icon", "path", [](Player& pl) { _plotShop(pl); });

    fm.appendButton("自定义设置", "textures/ui/gear", "path", [](Player& pl) { _playerSetting(pl); });
    fm.appendButton("插件设置\n(管理员)", "textures/ui/gear", "path", [](Player& pl) { _pluginSetting(pl); });

    fm.appendButton("退出", [](Player&) {});
    fm.sendTo(player);
}

void _playerSetting(Player& player) {
    CustomForm fm{PLUGIN_TITLE};

    auto setting     = data::PlotBDStorage::getInstance().getPlayerSetting(player.getUuid().asString());
    auto i18n        = ll::i18n::getInstance().get();
    auto settingJson = JsonHelper::structToJson(setting);

    for (auto const& [key, value] : settingJson.items()) {
        fm.appendToggle(key, string(i18n->get(key)), value.get<bool>());
    }

    fm.sendTo(player, [setting, settingJson](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }
        utils::DebugFormPrint(dt);

        json              setj = settingJson; // copy
        PlayerSettingItem it   = setting;

        for (auto const& [key, value] : setj.items()) {
            bool const val = std::get<uint64_t>(dt->at(key));
            setj[key]      = val;
        }

        JsonHelper::jsonToStruct(setj, it);

        data::PlotBDStorage::getInstance().setPlayerSetting(pl.getUuid().asString(), it);
        sendText(pl, "设置成功");
    });
}


void _plotShop(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 地皮商店(玩家出售)");

    auto* impl = &data::PlotBDStorage::getInstance();

    auto sls = impl->getSaleingPlots();
    for (auto const& sl : sls) {
        auto pt = impl->getPlot(sl->getPlotID());
        if (!pt) continue;
        if (pt->isOwner(player.getUuid().asString())) continue; // 跳过自己的地皮
        fm.appendButton(
            fmt::format("{}\nID: {} | 价格: {}", pt->getPlotName(), sl->getPlotID(), sl->getSalePrice()),
            [pt, sl](Player& pl) { _plotShopShowPlot(pl, pt.get()); }
        );
    }

    fm.sendTo(player);
}

void _plotShopShowPlot(Player& player, PlotMetadata* pt) {
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
        pl.teleport(pps.getSafestPos(), getPlotDimensionId());
    });

    fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { _plotShop(pl); });

    fm.sendTo(player);
}

void _pluginSetting(Player& player) {
    auto* impl = &data::PlotBDStorage::getInstance();
    auto* cfg  = &config::cfg.switchDim;

    if (!impl->isAdmin(player.getUuid().asString())) {
        sendText<utils::Level::Error>(player, "你没有权限执行此操作");
        return;
    }

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent("PlotCraft > 插件设置");

    fm.appendButton("设置当前位置为主世界安全坐标", "textures/ui/Wrenches1", "path", [cfg](Player& pl) {
        if (pl.getDimensionId() != 0) {
            sendText<utils::Level::Error>(pl, "你必须在主世界才能执行此操作");
            return;
        }
        auto const ps     = pl.getPosition();
        cfg->overWorld[0] = ps.x;
        cfg->overWorld[1] = ps.y;
        cfg->overWorld[2] = ps.z;
        config::updateConfig();
        sendText(pl, "设置成功");
    });

    fm.appendButton("设置当前位置为地皮世界安全坐标", "textures/ui/Wrenches1", "path", [cfg](Player& pl) {
        if (pl.getDimensionId() != getPlotDimensionId()) {
            sendText<utils::Level::Error>(pl, "你必须在地皮世界才能执行此操作");
            return;
        }
        auto const ps     = pl.getPosition();
        cfg->plotWorld[0] = ps.x;
        cfg->plotWorld[1] = ps.y;
        cfg->plotWorld[2] = ps.z;
        config::updateConfig();
        sendText(pl, "设置成功");
    });

    fm.sendTo(player);
}


void _selectPlot(Player& player) {
    SimpleForm fm{PLUGIN_TITLE};

    fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { index(pl); });


    auto plots = data::PlotBDStorage::getInstance().getPlots();

    for (auto const& plt : plots) {
        if (!plt->isOwner(player.getUuid().asString())) continue; // 不是自己的地皮
        fm.appendButton(fmt::format("{}\n{}", plt->getPlotName(), plt->getPlotID()), [plt](Player& pl) {
            plot(pl, plt.get(), true);
        });
    }

    fm.sendTo(player);
}


// command index
void plot(Player& player, PlotPos plotPos) {
    if (player.getDimensionId() != getPlotDimensionId()) {
        sendText<utils::Level::Error>(player, "你必须在地皮世界才能执行此操作");
        return;
    }

    std::shared_ptr<PlotMetadata> plot = data::PlotBDStorage::getInstance().getPlot(plotPos.getPlotID());

    if (plot == nullptr) {
        plot = PlotMetadata::make(plotPos.getPlotID(), plotPos.x, plotPos.z);
    }

    gui::plot(player, plot.get(), false);
}


void plot(Player& player, PlotMetadata* pt, bool ret) {
    SimpleForm fm{PLUGIN_TITLE};

    auto& ndb = PlayerNameDB::getInstance();
    auto& cfg = config::cfg;

    bool const hasOwner       = !pt->getPlotOwner().empty();                                   // 是否有主人
    bool const hasSale        = pt->isSale();                                                  // 是否出售
    bool const isOwner        = hasOwner && player.getUuid().asString() == pt->getPlotOwner(); // 是否是主人
    bool const isSharedMember = pt->isSharedPlayer(player.getUuid().asString()); // 是否是地皮共享成员

    fm.setContent(fmt::format(
        "地皮 {} 的元数据:\n地皮主人: {}\n地皮名称: {}\n是否出售: {}\n出售价格: {}\n  ",
        pt->getPlotID(),
        !hasOwner ? "无主" : ndb.getPlayerName(pt->getPlotOwner()),
        pt->getPlotName(),
        hasOwner ? hasSale ? "是" : "否" : "是",
        hasOwner ? hasSale ? std::to_string(pt->getSalePrice()) : "null"
                 : std::to_string(config::cfg.plotWorld.buyPlotPrice)
    ));


    if ((!hasOwner || hasSale) && !isOwner)
        fm.appendButton("购买地皮", "textures/ui/confirm", "path", [pt](Player& pl) { _buyPlot(pl, pt); });

    if ((isOwner || isSharedMember) && utils::some(cfg.allowedPlotTeleportDim, player.getDimensionId().id))
        fm.appendButton("传送到此地皮", "textures/ui/move", "path", [pt](Player& pl) {
            auto const v3 = PlotPos{pt->getX(), pt->getZ()}.getSafestPos();
            pl.teleport(v3, getPlotDimensionId());
            sendText(pl, "传送成功");
        });

    if (isOwner) {
        fm.appendButton("修改地皮名称", "textures/ui/book_edit_default", "path", [pt](Player& pl) {
            _changePlotName(pl, pt);
        });

        fm.appendButton("地皮出售", "textures/ui/MCoin", "path", [pt](Player& pl) { _sellMyPlot(pl, pt); });

        fm.appendButton("共享地皮", "textures/ui/share_microsoft", "path", [pt](Player& pl) {
            _plotShareManage(pl, pt);
        });
    }

    if (hasOwner)
        fm.appendButton("地皮评论", "textures/ui/icon_sign", "path", [pt](Player& pl) { _plotcomment(pl, pt); });

    if (ret) fm.appendButton("返回", "textures/ui/icon_import", "path", [](Player& pl) { _selectPlot(pl); });

    fm.sendTo(player);
}


void _sellMyPlot(Player& player, PlotMetadata* pt) {
    bool const isSaleing = pt->isSale();

    SimpleForm fm{PLUGIN_TITLE};

    if (isSaleing) {
        fm.setContent(fmt::format("你正在出售地皮 {}，价格为 {}。", pt->getPlotID(), pt->getSalePrice()));

        fm.appendButton("编辑出售价格", "textures/ui/book_edit_default", "path", [pt](Player& pl) {
            _sellPlotAndEditPrice(pl, pt, true);
        });
        fm.appendButton("取消出售", "textures/ui/cancel", "path", [pt](Player& pl) {
            bool const ok = pt->setSaleStatus(false, 0);
            if (ok) sendText(pl, "出售已取消");
            else sendText<utils::Level::Error>(pl, "出售取消失败");
        });
    } else {
        fm.setContent("此地皮没有出售，无法查询信息。");

        fm.appendButton("出售当前地皮", "textures/ui/icon_minecoin_9x9", "path", [pt](Player& pl) {
            _sellPlotAndEditPrice(pl, pt, false);
        });
    }

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { plot(pl, pt, true); });

    fm.sendTo(player);
}


void _sellPlotAndEditPrice(Player& player, PlotMetadata* pt, bool edit) {
    CustomForm fm{PLUGIN_TITLE};

    fm.appendLabel("地皮确认出售后，其它玩家可以查看到你的出售信息。当玩家购买后，你的地皮将被转移到购买者的名下("
                   "自动重置共享信息)。");

    if (edit) fm.appendInput("pr", "请输入出售价格:", "integer", std::to_string(pt->getSalePrice()));
    else fm.appendInput("pr", "请输入出售价格:", "integer");

    fm.sendTo(player, [pt, edit](Player& pl, CustomFormResult const& dt, FormCancelReason) {
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
            bool const ok = pt->setSalePrice(p);
            if (ok) sendText(pl, "出售价格已修改");
            else sendText<utils::Level::Error>(pl, "出售价格修改失败");
        } else {
            bool const ok = pt->setSaleStatus(true, p);
            if (ok) sendText(pl, "出售成功");
            else sendText<utils::Level::Error>(pl, "出售失败");
        }
    });
}


void _plotShareManage(Player& player, PlotMetadata* pt) {
    auto* ndb = &PlayerNameDB::getInstance();

    SimpleForm fm{PLUGIN_TITLE};
    fm.setContent("地皮共享设置\n将玩家设置为当前地皮共享者(信任者)后\n被授权的玩家拥有共享的地皮权限(放置、破坏、修改)"
                  "。注意：此权限不包含打开GUI修改地皮的权限。");

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { plot(pl, pt, true); });

    fm.appendButton("清除所有共享者", "textures/ui/recap_glyph_color_2x", "path", [pt](Player& pl) {
        bool const ok = pt->resetSharedPlayers();
        if (ok) sendText(pl, "共享信息已清除");
        else sendText<utils::Level::Error>(pl, "共享信息清除失败");
    });

    fm.appendButton("添加共享者", "textures/ui/color_plus", "path", [pt](Player& pl) { _addSharePlayer(pl, pt); });


    auto sharedInfos = pt->getSharedPlayers();
    for (auto const& si : sharedInfos) {
        fm.appendButton(
            fmt::format("{}\n{}", ndb->getPlayerName(si.mSharedPlayer), si.mSharedTime),
            [pt, si, ndb](Player& pl) {
                ModalForm{
                    PLUGIN_TITLE,
                    fmt::format("共享者: {}\n共享时间: {}", ndb->getPlayerName(si.mSharedPlayer), si.mSharedTime),
                    "删除此玩家的共享权限",
                    "返回"
                }
                    .sendTo(pl, [pt, si, ndb](Player& pl, ModalFormResult const& dt, FormCancelReason) {
                        if (!dt) {
                            sendText(pl, "表单已放弃");
                            return;
                        }
                        if (!(bool)dt.value()) {
                            plot(pl, pt, true);
                            return;
                        }

                        bool const ok = pt->delSharedPlayer(si.mSharedPlayer);
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

void _addSharePlayer(Player& player, PlotMetadata* pt) {
    auto* ndb = &PlayerNameDB::getInstance();

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

    fm.sendTo(player, [pt, ndb](Player& pl, CustomFormResult const& dt, FormCancelReason) {
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
            if (uuid.empty()) {
                sendText(pl, "输入的共享者不存在,获取UUID失败");
                return;
            }

            ok = pt->addSharedPlayer(uuid);
        } else {
            // 在线
            ok = pt->addSharedPlayer(ndb->getPlayerUUID(sp));
        }

        if (ok) sendText(pl, "共享权限添加成功");
        else sendText<utils::Level::Error>(pl, "共享权限添加失败");
    });
}


void _changePlotName(Player& player, PlotMetadata* pt) {
    auto* bus = &ll::event::EventBus::getInstance();

    pev::PlayerChangePlotNameBefore ev{&player, pt};
    bus->publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("pn", "地皮名称:", "string", pt->getPlotName());

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

        bool const ok = pt->setPlotName(pn);

        if (ok) {
            pev::PlayerChangePlotNameAfter ev{&pl, pt, pn};
            bus->publish(ev);

            sendText(pl, "地皮名称已修改");
        } else {
            sendText<utils::Level::Error>(pl, "地皮名称修改失败");
        }
    });
}


void _buyPlot(Player& player, PlotMetadata* pt) {

    auto* impl = &data::PlotBDStorage::getInstance();
    auto& cfg  = config::cfg.plotWorld;

    if (static_cast<int>(impl->getPlots(player.getUuid().asString()).size()) >= cfg.maxBuyPlotCount) {
        sendText<utils::Level::Warn>(player, "你已经购买了太多地皮，无法购买新的地皮。");
        return;
    }

    auto*      ms       = &Moneys::getInstance();
    bool const hasOwner = !pt->getPlotOwner().empty();
    bool const hasSale  = pt->isSale();
    bool const fromSale = hasOwner && hasSale; // 是否从出售地皮购买(玩家)
    int const  price    = fromSale ? pt->getSalePrice() : cfg.buyPlotPrice;

    if (hasOwner && !hasSale) {
        sendText<utils::Level::Warn>(player, "这个地皮没有出售，无法购买。");
        return;
    }
    if (pt->getPlotOwner() == player.getUuid().asString()) {
        sendText<utils::Level::Warn>(player, "你不能购买自己的地皮。");
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
                plot(pl, pt, true);
                return;
            }

            if (ms->reduceMoney(pl, price)) {
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
                            ms->addMoney(plptr, newPrice); // 出售者(当前地皮主人)在线
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
                    } else sendText<utils::Level::Error>(pl, "地皮购买失败");
                } else {
                    PlotPos    ps{pt->getX(), pt->getZ()};
                    bool const ok = impl->addPlot(ps.getPlotID(), pl.getUuid().asString(), pt->getX(), pt->getZ());
                    if (ok) {
                        bus.publish(ev);
                        sendText(pl, "地皮购买成功");
                    } else sendText<utils::Level::Error>(pl, "地皮购买失败");
                }
            }
        });
}


void _plotcomment(Player& player, PlotMetadata* pt) {

    bool const hasOwner = !pt->getPlotOwner().empty(); // 是否有主人

    if (!hasOwner) {
        sendText<utils::Level::Warn>(player, "你不能评论这个地皮，因为它没有主人。");
        return;
    }

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent(fmt::format("地皮 '{}' 的评论区:", pt->getPlotID()));

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { plot(pl, pt, true); });

    fm.appendButton("发表评论", "textures/ui/message", "path", [pt](Player& pl) { _publishComment(pl, pt); });


    auto& ndb = PlayerNameDB::getInstance();
    auto  cts = pt->getComments();

    std::sort(cts.begin(), cts.end(), [](auto const& a, auto const& b) {
        return Date::parse(a.mCommentTime) < Date::parse(b.mCommentTime);
    });

    for (auto const& ct : cts) {
        fm.appendButton(
            fmt::format("{}  {}\n{}", ndb.getPlayerName(ct.mCommentPlayer), ct.mCommentTime, ct.mContent),
            [ct, pt](Player& pl) { _showCommentOperation(pl, pt, ct.mCommentID); }
        );
    }

    fm.sendTo(player);
}


void _publishComment(Player& player, PlotMetadata* pt) {
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

        bool const ok = pt->addComment(pl.getUuid().asString(), ct);

        if (ok) {
            pev::PlayerCommentPlotAfter ev{&pl, pt, ct};
            ll::event::EventBus::getInstance().publish(ev);

            sendText(pl, "评论已发布");
        } else {
            sendText<utils::Level::Error>(pl, "评论发布失败");
        }
    });
}


void _showCommentOperation(Player& player, PlotMetadata* pt, CommentID id) {
    auto const ct             = *pt->getComment(id);
    bool const isOwner        = player.getUuid().asString() == pt->getPlotOwner();
    bool const isCommentOwner = player.getUuid().asString() == ct.mCommentPlayer;

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
        fm.appendButton("编辑评论", "textures/ui/book_edit_default", "path", [pt, id](Player& pl) {
            _editComment(pl, pt, id);
        });
    }

    if (isCommentOwner || isOwner) {
        fm.appendButton("删除评论", "textures/ui/icon_trash", "path", [pt, id](Player& pl) {
            pev::PlayerDeletePlotComment ev{&pl, pt, id};
            ll::event::EventBus::getInstance().publish(ev);
            if (ev.isCancelled()) return; // 事件被取消

            bool const ok = pt->delComment(id);

            if (ok) {
                sendText(pl, "评论已删除");
            } else {
                sendText<utils::Level::Error>(pl, "评论删除失败");
            }
        });
    }

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { _plotcomment(pl, pt); });

    fm.sendTo(player);
}


void _editComment(Player& player, PlotMetadata* pt, CommentID id) {
    pev::PlayerEditCommentBefore ev{&player, pt, id};
    ll::event::EventBus::getInstance().publish(ev);
    if (ev.isCancelled()) return; // 事件被取消

    auto const& ct = *pt->getComment(id);

    CustomForm fm{PLUGIN_TITLE};

    fm.appendInput("cm", "评论内容:", "string", ct.mContent);

    fm.sendTo(player, [pt, id](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) {
            sendText(pl, "表单已放弃");
            return;
        }

        string cm = std::get<string>(dt->at("cm"));

        if (cm.empty()) {
            sendText(pl, "评论内容不能为空");
            return;
        }

        bool const ok = pt->setCommentContent(id, cm);

        if (ok) {
            pev::PlayerEditCommentAfter ev{&pl, pt, id, cm};
            ll::event::EventBus::getInstance().publish(ev);

            sendText(pl, "评论已修改");
        } else {
            sendText<utils::Level::Error>(pl, "评论修改失败");
        }
    });
}


} // namespace plo::gui