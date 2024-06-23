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
PlayerCommentPlotBefore::PlayerCommentPlotBefore(Player* player, Plot pt) : mPlayer(player), mPlot(pt) {}
Player* PlayerCommentPlotBefore::getPlayer() const { return mPlayer; }
Plot    PlayerCommentPlotBefore::getPlot() const { return mPlot; }


// 玩家评论地皮之后
PlayerCommentPlotAfter::PlayerCommentPlotAfter(Player* player, Plot plot, string content)
: mPlayer(player),
  mPlot(plot),
  mContent(content) {}
Player* PlayerCommentPlotAfter::getPlayer() const { return mPlayer; }
Plot    PlayerCommentPlotAfter::getPlot() const { return mPlot; }
string  PlayerCommentPlotAfter::getContent() const { return mContent; }


// 玩家编辑评论之前
PlayerEditCommentBefore::PlayerEditCommentBefore(Player* player, Plot plot, PlotComment comment)
: mPlayer(player),
  mPlot(plot),
  mComment(comment) {}
Player*     PlayerEditCommentBefore::getPlayer() const { return mPlayer; }
Plot        PlayerEditCommentBefore::getPlot() const { return mPlot; }
PlotComment PlayerEditCommentBefore::getComment() const { return mComment; }


// 玩家编辑评论之后
PlayerEditCommentAfter::PlayerEditCommentAfter(Player* player, Plot plot, PlotComment comment, string newContent)
: mPlayer(player),
  mPlot(plot),
  mComment(comment),
  mNewContent(newContent) {}
Player*     PlayerEditCommentAfter::getPlayer() const { return mPlayer; }
Plot        PlayerEditCommentAfter::getPlot() const { return mPlot; }
PlotComment PlayerEditCommentAfter::getComment() const { return mComment; }
string      PlayerEditCommentAfter::getNewContent() const { return mNewContent; }


// 玩家删除评论
PlayerDeletePlotComment::PlayerDeletePlotComment(Player* player, Plot plot, PlotComment comment)
: mPlayer(player),
  mPlot(plot),
  mComment(comment) {}
Player*     PlayerDeletePlotComment::getPlayer() const { return mPlayer; }
Plot        PlayerDeletePlotComment::getPlot() const { return mPlot; }
PlotComment PlayerDeletePlotComment::getComment() const { return mComment; }


// 玩家购买地皮之前
PlayerBuyPlotBefore::PlayerBuyPlotBefore(Player* player, Plot plot, int price)
: mPlayer(player),
  mPlot(plot),
  mPrice(price) {}
Player* PlayerBuyPlotBefore::getPlayer() const { return mPlayer; }
Plot    PlayerBuyPlotBefore::getPlot() const { return mPlot; }
int     PlayerBuyPlotBefore::getPrice() const { return mPrice; }


// 玩家购买地皮之后
PlayerBuyPlotAfter::PlayerBuyPlotAfter(Player* player, Plot plot, int price)
: mPlayer(player),
  mPlot(plot),
  mPrice(price) {}
Player* PlayerBuyPlotAfter::getPlayer() const { return mPlayer; }
Plot    PlayerBuyPlotAfter::getPlot() const { return mPlot; }
int     PlayerBuyPlotAfter::getPrice() const { return mPrice; }


// 玩家修改地皮名称之前
PlayerChangePlotNameBefore::PlayerChangePlotNameBefore(Player* player, Plot plot) : mPlayer(player), mPlot(plot) {}
Player* PlayerChangePlotNameBefore::getPlayer() const { return mPlayer; }
Plot    PlayerChangePlotNameBefore::getPlot() const { return mPlot; }


// 玩家修改地皮名称之后
PlayerChangePlotNameAfter::PlayerChangePlotNameAfter(Player* player, Plot plot, string newName)
: mPlayer(player),
  mPlot(plot),
  mNewName(newName) {}
Player* PlayerChangePlotNameAfter::getPlayer() const { return mPlayer; }
Plot    PlayerChangePlotNameAfter::getPlot() const { return mPlot; }
string  PlayerChangePlotNameAfter::getNewName() const { return mNewName; }

} // namespace plo::event