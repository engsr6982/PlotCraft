#pragma once
#include "mc/deps/core/mce/UUID.h"
#include "plotcraft/Macro.h"
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
struct PlotPermissionTable {};


class PlotMetadata {
public:
    int version = 1; // Metadata版本(用于反射合并冲突)

    // private:
    PlotID mPlotID;
    string mPlotName  = "";
    UUID   mPlotOwner = "";
    int    mPlotX;
    int    mPlotZ;

    bool mIsSale = false; // 是否出售
    int  mPrice  = 0;     // 出售价格

    PlotPermissionTable mPermissionTable; // 权限表

    std::vector<PlotShareItem> mSharedPlayers; // 共享者列表

    std::vector<PlotCommentItem> mComments; // 评论列表

public:
    // Constructors:
    PLAPI static std::shared_ptr<PlotMetadata> make();
    PLAPI static std::shared_ptr<PlotMetadata> make(PlotID const& plotID, int x, int z);
    PLAPI static std::shared_ptr<PlotMetadata> make(PlotID const& plotID, UUID const& owner, int x, int z);
    PLAPI static std::shared_ptr<PlotMetadata>
    make(PlotID const& plotID, UUID const& owner, string const& name, int x, int z);


    // APIs:
    PLAPI bool isOwner(UUID const& uuid) const;

    PLAPI bool setPlotName(string const& name);

    PLAPI bool setPlotID(PlotID const& plotID);

    PLAPI bool setX(int x);

    PLAPI bool setZ(int z);

    PLAPI bool setPlotOwner(UUID const& owner);

    PLAPI bool isSharedPlayer(UUID const& uuid) const;

    PLAPI bool addSharedPlayer(UUID const& uuid);

    PLAPI bool delSharedPlayer(UUID const& uuid);

    PLAPI bool resetSharedPlayers();

    PLAPI std::vector<PlotShareItem> getSharedPlayers() const;

    PLAPI bool hasComment(CommentID const& commentID) const;

    PLAPI bool isCommentOwner(CommentID const& commentID, UUID const& uuid) const;

    PLAPI bool addComment(UUID const& uuid, string const& content);

    PLAPI bool delComment(CommentID const& commentID);

    PLAPI bool resetComments();

    PLAPI bool setCommentContent(CommentID const& commentID, string const& content);

    PLAPI std::optional<PlotCommentItem> getComment(CommentID const& commentID) const;
    PLAPI std::vector<PlotCommentItem> getComments() const;
    PLAPI std::vector<PlotCommentItem> getComments(UUID const& uuid) const;

    PLAPI bool setSaleStatus(bool isSale);

    PLAPI bool setSaleStatus(bool isSale, int price);

    PLAPI bool setSalePrice(int price);

    PLAPI bool isSale() const;

    PLAPI int getSalePrice() const;

    PLAPI PlotID getPlotID() const;

    PLAPI string getPlotName() const;

    PLAPI UUID getPlotOwner() const;

    PLAPI int getX() const;

    PLAPI int getZ() const;

    PLAPI PlotPermission getPlayerInThisPlotPermission(UUID const& uuid) const; // 玩家在此地皮的权限

    PLAPI PlotPermissionTable&       getPermissionTable();
    PLAPI PlotPermissionTable const& getPermissionTable() const;

    PLAPI void save();

    PLAPI string toString() const;
};


} // namespace plo::data
