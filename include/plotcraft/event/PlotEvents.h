#pragma once
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/Event.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"
#include "plotcraft/core/PPos.h"
#include "plotcraft/data/PlotMetadata.h"

namespace plo::event {

using namespace data;

// 玩家进入地皮
class PlayerEnterPlot final : public ll::event::Event {
private:
    PPos const& mPos;    // 地皮坐标
    Player*     mPlayer; // 玩家指针

public:
    constexpr explicit PlayerEnterPlot(PPos const& pos, Player* player) : mPos(pos), mPlayer(player) {}

    PLAPI Player*     getPlayer() const;
    PLAPI PPos const& getPos() const;
};


// 玩家离开地皮
class PlayerLeavePlot final : public ll::event::Event {
private:
    PPos const& mPos;    // 地皮坐标
    Player*     mPlayer; // 玩家指针

public:
    constexpr explicit PlayerLeavePlot(const PPos& pos, Player* player) : mPos(pos), mPlayer(player) {}

    PLAPI Player*     getPlayer() const;
    PLAPI PPos const& getPos() const;
};


// 玩家评论地皮前(可拦截)
class PlayerCommentPlotBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;

public:
    constexpr explicit PlayerCommentPlotBefore(Player* player, PlotMetadataPtr const& plot)
    : Cancellable(),
      mPlayer(player),
      mPlot(plot) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
};


// 玩家评论地皮后
class PlayerCommentPlotAfter final : public ll::event::Event {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    string const&          mContent; // 评论内容

public:
    constexpr explicit PlayerCommentPlotAfter(Player* player, PlotMetadataPtr const& plot, string const& content)
    : mPlayer(player),
      mPlot(plot),
      mContent(content) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI string const&          getContent() const;
};

// 玩家编辑评论前(可拦截)
class PlayerEditCommentBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    CommentID const&       mCommentID;

public:
    constexpr explicit PlayerEditCommentBefore(Player* player, PlotMetadataPtr const& plot, CommentID const& comment)
    : Cancellable(),
      mPlayer(player),
      mPlot(plot),
      mCommentID(comment) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI CommentID const&       getCommentID() const;
};

// 玩家编辑评论后
class PlayerEditCommentAfter final : public ll::event::Event {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    CommentID const&       mCommentID;
    string const&          mNewContent; // 评论内容

public:
    constexpr explicit PlayerEditCommentAfter(
        Player*                player,
        PlotMetadataPtr const& plot,
        CommentID const&       comment,
        string const&          newContent
    )
    : mPlayer(player),
      mPlot(plot),
      mCommentID(comment),
      mNewContent(newContent) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI CommentID const&       getCommentID() const;
    PLAPI string const&          getNewContent() const;
};

// 玩家删除地皮评论(可拦截)
class PlayerDeletePlotComment final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    CommentID const&       mCommentID;

public:
    constexpr explicit PlayerDeletePlotComment(Player* player, PlotMetadataPtr const& plot, CommentID const& comment)
    : Cancellable(),
      mPlayer(player),
      mPlot(plot),
      mCommentID(comment) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI CommentID const&       getCommentID() const;
};


// 玩家购买地皮前(可拦截)
class PlayerBuyPlotBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    int const&             mPrice;

public:
    constexpr explicit PlayerBuyPlotBefore(Player* player, PlotMetadataPtr const& plot, int const& price)
    : Cancellable(),
      mPlayer(player),
      mPlot(plot),
      mPrice(price) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI int const&             getPrice() const;
};

// 玩家购买地皮后
class PlayerBuyPlotAfter final : public ll::event::Event {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    int const&             mPrice;

public:
    constexpr explicit PlayerBuyPlotAfter(Player* player, PlotMetadataPtr const& plot, int const& price)
    : mPlayer(player),
      mPlot(plot),
      mPrice(price) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI int const&             getPrice() const;
};

// 玩家修改地皮名称前(可拦截)
class PlayerChangePlotNameBefore final : public ll::event::Cancellable<ll::event::Event> {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;

public:
    constexpr explicit PlayerChangePlotNameBefore(Player* player, PlotMetadataPtr const& plot)
    : Cancellable(),
      mPlayer(player),
      mPlot(plot) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
};

// 玩家修改地皮名称后
class PlayerChangePlotNameAfter final : public ll::event::Event {
private:
    Player*                mPlayer;
    PlotMetadataPtr const& mPlot;
    string const&          mNewName;

public:
    constexpr explicit PlayerChangePlotNameAfter(Player* player, PlotMetadataPtr const& plot, string const& newName)
    : mPlayer(player),
      mPlot(plot),
      mNewName(newName) {}

    PLAPI Player*                getPlayer() const;
    PLAPI PlotMetadataPtr const& getPlot() const;
    PLAPI string const&          getNewName() const;
};

} // namespace plo::event