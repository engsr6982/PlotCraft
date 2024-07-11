#pragma once
#include "mc/deps/core/mce/UUID.h"
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using string = std::string;


namespace plo::data {

typedef string    PlotID; // PlotPos::toString()
typedef mce::UUID UUID_;
typedef string    UUID;
typedef int       CommentID;


enum class PlotPermission : int { None = 0, Shared = 1, Owner = 2, Admin = 3 };


struct PlotCommentItem {
    CommentID mCommentID;
    UUID      mCommentPlayer;
    string    mCommentTime;
    string    mContent;
};
struct PlotShareItem {
    UUID   mSharedPlayer;
    string mSharedTime;
};


class PlotMetadata {
public: // private:
    PlotID mPlotID;
    string mPlotName  = "";
    UUID   mPlotOwner = "";
    int    mPlotX;
    int    mPlotZ;

    bool mIsSale = false; // 是否是出售的
    int  mPrice  = 0;     // 出售价格

    std::vector<PlotShareItem> mSharedPlayers; // 共享者列表

    std::vector<PlotCommentItem> mComments; // 评论列表

public:
    // PlotMetadata();
    // PlotMetadata(PlotID const& plotID, UUID const& owner, int x, int z);

    // PlotMetadata(const PlotMetadata&)            = delete;
    // PlotMetadata& operator=(const PlotMetadata&) = delete;
    // PlotMetadata(PlotMetadata&&)                 = delete;
    // PlotMetadata& operator=(PlotMetadata&&)      = delete;


    static std::shared_ptr<PlotMetadata> make(PlotID const& plotID, UUID const& owner, int x, int z);

    static std::shared_ptr<PlotMetadata>
    make(PlotID const& plotID, UUID const& owner, string const& name, int x, int z);

    static std::shared_ptr<PlotMetadata> make(PlotID const& plotID, int x, int z);

    static std::shared_ptr<PlotMetadata> make();

    // APIs:
    bool isOwner(UUID const& uuid) const;

    bool setPlotName(string const& name);

    bool setPlotID(PlotID const& plotID);

    bool setX(int x);

    bool setZ(int z);

    bool setPlotOwner(UUID const& owner);

    bool isSharedPlayer(UUID const& uuid) const;

    bool addSharedPlayer(UUID const& uuid);

    bool delSharedPlayer(UUID const& uuid);

    bool resetSharedPlayers();

    std::vector<PlotShareItem> getSharedPlayers() const;

    bool hasComment(CommentID const& commentID) const;

    bool isCommentOwner(CommentID const& commentID, UUID const& uuid) const;

    bool addComment(UUID const& uuid, string const& content);

    bool delComment(CommentID const& commentID);

    bool resetComments();

    bool setCommentContent(CommentID const& commentID, string const& content);

    std::optional<PlotCommentItem> getComment(CommentID const& commentID) const;
    std::vector<PlotCommentItem>   getComments() const;
    std::vector<PlotCommentItem>   getComments(UUID const& uuid) const;

    bool setSaleStatus(bool isSale);

    bool setSaleStatus(bool isSale, int price);

    bool setSalePrice(int price);

    bool isSale() const;

    int getSalePrice() const;

    PlotID getPlotID() const;

    string getPlotName() const;

    UUID getPlotOwner() const;

    int getX() const;

    int getZ() const;

    PlotPermission getPlayerInThisPlotPermission(UUID const& uuid) const; // 玩家在此地皮的权限

    void save();

    string toString() const;
};


} // namespace plo::data
