#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/data/PlotMetadata.h"

namespace plo::event {

using namespace data;

// 玩家进入地皮
class PlayerEnterPlot final : public ll::event::Event {
private:
    PlotPos mPos;    // 地皮坐标
    Player* mPlayer; // 玩家指针

public:
    PLAPI PlayerEnterPlot(const PlotPos& pos, Player* player);

    PLAPI Player* getPlayer() const;
    PLAPI PlotPos getPos() const;
};


// 玩家离开地皮
class PlayerLeavePlot final : public ll::event::Event {
private:
    PlotPos mPos;    // 地皮坐标
    Player* mPlayer; // 玩家指针

public:
    PLAPI PlayerLeavePlot(const PlotPos& pos, Player* player);

    PLAPI Player* getPlayer() const;
    PLAPI PlotPos getPos() const;
};


// 玩家评论地皮前(可拦截)
class PlayerCommentPlotBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;

public:
    PLAPI PlayerCommentPlotBefore(Player* player, const PlotMetadata* plot);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
};


// 玩家评论地皮后
class PlayerCommentPlotAfter final : public ll::event::Event {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    string              mContent; // 评论内容

public:
    PLAPI PlayerCommentPlotAfter(Player* player, const PlotMetadata* plot, string content);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI string              getContent() const;
};

// 玩家编辑评论前(可拦截)
class PlayerEditCommentBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    CommentID           mCommentID;

public:
    PLAPI PlayerEditCommentBefore(Player* player, const PlotMetadata* plot, CommentID comment);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI CommentID           getCommentID() const;
};

// 玩家编辑评论后
class PlayerEditCommentAfter final : public ll::event::Event {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    CommentID           mCommentID;
    string              mNewContent; // 评论内容

public:
    PLAPI
    PlayerEditCommentAfter(Player* player, const PlotMetadata* plot, CommentID comment, string newContent);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI CommentID           getCommentID() const;
    PLAPI string              getNewContent() const;
};

// 玩家删除地皮评论(可拦截)
class PlayerDeletePlotComment final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    CommentID           mCommentID;

public:
    PLAPI PlayerDeletePlotComment(Player* player, const PlotMetadata* plot, CommentID comment);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI CommentID           getCommentID() const;
};


// 玩家购买地皮前(可拦截)
class PlayerBuyPlotBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    int                 mPrice;

public:
    PLAPI PlayerBuyPlotBefore(Player* player, const PlotMetadata* plot, int price);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI int                 getPrice() const;
};

// 玩家购买地皮后
class PlayerBuyPlotAfter final : public ll::event::Event {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    int                 mPrice;

public:
    PLAPI PlayerBuyPlotAfter(Player* player, const PlotMetadata* plot, int price);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI int                 getPrice() const;
};

// 玩家修改地皮名称前(可拦截)
class PlayerChangePlotNameBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;

public:
    PLAPI PlayerChangePlotNameBefore(Player* player, const PlotMetadata* plot);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
};

// 玩家修改地皮名称后
class PlayerChangePlotNameAfter final : public ll::event::Event {
private:
    Player*             mPlayer;
    const PlotMetadata* mPlot;
    string              mNewName;

public:
    PLAPI PlayerChangePlotNameAfter(Player* player, const PlotMetadata* plot, string newName);

    PLAPI Player*             getPlayer() const;
    PLAPI const PlotMetadata* getPlot() const;
    PLAPI string              getNewName() const;
};

} // namespace plo::event