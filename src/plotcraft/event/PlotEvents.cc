#include "plotcraft/event/PlotEvents.h"

namespace plo::event {

// 玩家进入地皮
PlayerEnterPlot::PlayerEnterPlot(const PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}
Player* PlayerEnterPlot::getPlayer() const { return mPlayer; }
PlotPos PlayerEnterPlot::getPos() const { return mPos; }


// 玩家离开地皮
PlayerLeavePlot::PlayerLeavePlot(const PlotPos& pos, Player* player) : mPos(pos), mPlayer(player) {}
Player* PlayerLeavePlot::getPlayer() const { return mPlayer; }
PlotPos PlayerLeavePlot::getPos() const { return mPos; }


// 玩家评论地皮之前
PlayerCommentPlotBefore::PlayerCommentPlotBefore(Player* player, const PlotMetadataPtr pt)
: mPlayer(player),
  mPlot(pt) {}
Player*               PlayerCommentPlotBefore::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerCommentPlotBefore::getPlot() const { return mPlot; }


// 玩家评论地皮之后
PlayerCommentPlotAfter::PlayerCommentPlotAfter(Player* player, const PlotMetadataPtr plot, string content)
: mPlayer(player),
  mPlot(plot),
  mContent(content) {}
Player*               PlayerCommentPlotAfter::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerCommentPlotAfter::getPlot() const { return mPlot; }
string                PlayerCommentPlotAfter::getContent() const { return mContent; }


// 玩家编辑评论之前
PlayerEditCommentBefore::PlayerEditCommentBefore(Player* player, const PlotMetadataPtr plot, CommentID comment)
: mPlayer(player),
  mPlot(plot),
  mCommentID(comment) {}
Player*               PlayerEditCommentBefore::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerEditCommentBefore::getPlot() const { return mPlot; }
CommentID             PlayerEditCommentBefore::getCommentID() const { return mCommentID; }


// 玩家编辑评论之后
PlayerEditCommentAfter::PlayerEditCommentAfter(
    Player*               player,
    const PlotMetadataPtr plot,
    CommentID             comment,
    string                newContent
)
: mPlayer(player),
  mPlot(plot),
  mCommentID(comment),
  mNewContent(newContent) {}
Player*               PlayerEditCommentAfter::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerEditCommentAfter::getPlot() const { return mPlot; }
CommentID             PlayerEditCommentAfter::getCommentID() const { return mCommentID; }
string                PlayerEditCommentAfter::getNewContent() const { return mNewContent; }


// 玩家删除评论
PlayerDeletePlotComment::PlayerDeletePlotComment(Player* player, const PlotMetadataPtr plot, CommentID comment)
: mPlayer(player),
  mPlot(plot),
  mCommentID(comment) {}
Player*               PlayerDeletePlotComment::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerDeletePlotComment::getPlot() const { return mPlot; }
CommentID             PlayerDeletePlotComment::getCommentID() const { return mCommentID; }


// 玩家购买地皮之前
PlayerBuyPlotBefore::PlayerBuyPlotBefore(Player* player, const PlotMetadataPtr plot, int price)
: mPlayer(player),
  mPlot(plot),
  mPrice(price) {}
Player*               PlayerBuyPlotBefore::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerBuyPlotBefore::getPlot() const { return mPlot; }
int                   PlayerBuyPlotBefore::getPrice() const { return mPrice; }


// 玩家购买地皮之后
PlayerBuyPlotAfter::PlayerBuyPlotAfter(Player* player, const PlotMetadataPtr plot, int price)
: mPlayer(player),
  mPlot(plot),
  mPrice(price) {}
Player*               PlayerBuyPlotAfter::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerBuyPlotAfter::getPlot() const { return mPlot; }
int                   PlayerBuyPlotAfter::getPrice() const { return mPrice; }


// 玩家修改地皮名称之前
PlayerChangePlotNameBefore::PlayerChangePlotNameBefore(Player* player, const PlotMetadataPtr plot)
: mPlayer(player),
  mPlot(plot) {}
Player*               PlayerChangePlotNameBefore::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerChangePlotNameBefore::getPlot() const { return mPlot; }


// 玩家修改地皮名称之后
PlayerChangePlotNameAfter::PlayerChangePlotNameAfter(Player* player, const PlotMetadataPtr plot, string newName)
: mPlayer(player),
  mPlot(plot),
  mNewName(newName) {}
Player*               PlayerChangePlotNameAfter::getPlayer() const { return mPlayer; }
const PlotMetadataPtr PlayerChangePlotNameAfter::getPlot() const { return mPlot; }
string                PlayerChangePlotNameAfter::getNewName() const { return mNewName; }

} // namespace plo::event