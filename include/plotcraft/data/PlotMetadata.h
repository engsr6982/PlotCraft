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


typedef string    PlotID; // PlotPos::getPlotID()
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

#define METADATA_VERSION 3
struct PlotPermissionTable {
    bool canDestroyBlock{false};  // 破坏方块
    bool canPlaceBlock{false};    // 放置方块
    bool canUseItemOn{true};      // 使用物品(右键)
    bool canFireSpread{false};    // 火焰蔓延
    bool canAttack{true};         // 攻击
    bool canPickupItem{true};     // 拾取物品
    bool canInteractBlock{false}; // 与方块交互

    // LSE
    bool canFarmLandDecay{true};        // 耕地退化
    bool canOperateFrame{false};        // 操作展示框
    bool canMobHurt{false};             // 生物受伤
    bool canAttackBlock{false};         // 攻击方块
    bool canOperateArmorStand{false};   // 操作盔甲架
    bool canDropItem{true};             // 丢弃物品
    bool canStepOnPressurePlate{false}; // 踩压压力板
    bool canRide{false};                // 骑乘
    bool canWitherDestroyBlock{false};  // 凋零破坏方块
    bool canRedStoneUpdate{true};       // 红石更新
};


using PlotMetadataPtr = std::shared_ptr<class PlotMetadata>;
class PlotMetadata {
public:
    int version = METADATA_VERSION;

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
    PLAPI static PlotMetadataPtr make();
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUID const& owner, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUID const& owner, string const& name, int x, int z);


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
    PLAPI PlotPermissionTable const& getPermissionTableConst() const;

    PLAPI void save();

    PLAPI string toString() const;
};


} // namespace plo::data
