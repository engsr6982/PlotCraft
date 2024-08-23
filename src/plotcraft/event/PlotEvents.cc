#include "plotcraft/event/PlotEvents.h"

namespace plo::event {

// 玩家进入地皮
Player*     PlayerEnterPlot::getPlayer() const { return mPlayer; }
PPos const& PlayerEnterPlot::getPos() const { return mPos; }


// 玩家离开地皮
Player*     PlayerLeavePlot::getPlayer() const { return mPlayer; }
PPos const& PlayerLeavePlot::getPos() const { return mPos; }


// 玩家评论地皮之前
Player*                PlayerCommentPlotBefore::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerCommentPlotBefore::getPlot() const { return mPlot; }


// 玩家评论地皮之后
Player*                PlayerCommentPlotAfter::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerCommentPlotAfter::getPlot() const { return mPlot; }
string const&          PlayerCommentPlotAfter::getContent() const { return mContent; }


// 玩家编辑评论之前
Player*                PlayerEditCommentBefore::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerEditCommentBefore::getPlot() const { return mPlot; }
CommentID const&       PlayerEditCommentBefore::getCommentID() const { return mCommentID; }


// 玩家编辑评论之后
Player*                PlayerEditCommentAfter::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerEditCommentAfter::getPlot() const { return mPlot; }
CommentID const&       PlayerEditCommentAfter::getCommentID() const { return mCommentID; }
string const&          PlayerEditCommentAfter::getNewContent() const { return mNewContent; }


// 玩家删除评论
Player*                PlayerDeletePlotComment::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerDeletePlotComment::getPlot() const { return mPlot; }
CommentID const&       PlayerDeletePlotComment::getCommentID() const { return mCommentID; }


// 玩家购买地皮之前
Player*                PlayerBuyPlotBefore::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerBuyPlotBefore::getPlot() const { return mPlot; }
int const&             PlayerBuyPlotBefore::getPrice() const { return mPrice; }


// 玩家购买地皮之后
Player*                PlayerBuyPlotAfter::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerBuyPlotAfter::getPlot() const { return mPlot; }
int const&             PlayerBuyPlotAfter::getPrice() const { return mPrice; }


// 玩家修改地皮名称之前
Player*                PlayerChangePlotNameBefore::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerChangePlotNameBefore::getPlot() const { return mPlot; }


// 玩家修改地皮名称之后
Player*                PlayerChangePlotNameAfter::getPlayer() const { return mPlayer; }
PlotMetadataPtr const& PlayerChangePlotNameAfter::getPlot() const { return mPlot; }
string const&          PlayerChangePlotNameAfter::getNewName() const { return mNewName; }

} // namespace plo::event