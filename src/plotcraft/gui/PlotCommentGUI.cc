#include "Global.h"


namespace plo::gui {


void PlotCommentGUI(Player& player, PlotMetadataPtr pt) {

    bool const hasOwner = !pt->getPlotOwner().empty(); // 是否有主人

    if (!hasOwner) {
        sendText<utils::Level::Warn>(player, "你不能评论这个地皮，因为它没有主人。");
        return;
    }

    SimpleForm fm{PLUGIN_TITLE};

    fm.setContent(fmt::format("地皮 '{}' 的评论区:", pt->getPlotID()));

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { PlotGUI(pl, pt, true); });

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


void _publishComment(Player& player, PlotMetadataPtr pt) {
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


void _showCommentOperation(Player& player, PlotMetadataPtr pt, CommentID id) {
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

    fm.appendButton("返回", "textures/ui/icon_import", "path", [pt](Player& pl) { PlotCommentGUI(pl, pt); });

    fm.sendTo(player);
}


void _editComment(Player& player, PlotMetadataPtr pt, CommentID id) {
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