#pragma once
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include "ll/api/data/KeyValueDB.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"
#include "plotcraft/PlotPos.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


using string = std::string;

namespace plo::database {


// typedefs
typedef string    PlotID; // PlotPos::toString()
typedef mce::UUID UUID;


// Class: PlayerNameDB
class PlayerNameDB {
private:
    std::unique_ptr<ll::data::KeyValueDB> mPlayerNameDB;
    bool                                  isInit = false;

    PlayerNameDB()                               = default;
    PlayerNameDB(const PlayerNameDB&)            = delete;
    PlayerNameDB& operator=(const PlayerNameDB&) = delete;

public:
    PLAPI static PlayerNameDB& getInstance();
    PLAPI bool                 initPlayerNameDB();

    PLAPI bool hasPlayer(string const& realName);
    PLAPI bool hasPlayer(mce::UUID const& uuid);

    PLAPI std::optional<string> getPlayerName(mce::UUID const& uuid);
    PLAPI std::optional<mce::UUID> getPlayerUUID(string const& realName);

    PLAPI bool insertPlayer(Player& player);
};


// Table Structure
enum class PlotPermission : int {
    None   = 0, // 无权限
    Shared = 1, // 共享者
    Owner  = 2, // 主人
    Admin  = 3  // 管理员
};

struct PlotAdmin {
    UUID mUUID; // 主键
};
using PlotAdmins = std::vector<PlotAdmin>;

struct Plot {
    PlotID mPlotID; // 主键
    string mPlotName;
    UUID   mPlotOwner; // 主键
    int    mPlotX;
    int    mPlotZ;
};
using Plots = std::vector<Plot>;

struct PlotShare {
    PlotID mPlotID;       // 主键
    UUID   mSharedPlayer; // 主键
    string mSharedTime;
};
using PlotShares = std::vector<PlotShare>;

typedef int CommentID;
struct PlotComment {
    CommentID mCommentID;     // 主键
    PlotID    mPlotID;        // 主键
    UUID      mCommentPlayer; // 主键
    string    mCommentTime;
    string    mContent;
};
using PlotCommenets = std::vector<PlotComment>;

struct PlotSale {
    PlotID mPlotID; // 主键
    int    mPrice;
    string mSaleTime;
};
using PlotSales = std::vector<PlotSale>;


// Class: PlotDBImpl
class PlotDBImpl {
private:
    std::unique_ptr<SQLite::Database> mSQLite;
    bool                              isInit = false;


    PLAPI void initTables();
    PLAPI bool initSQLite();

public:
    PlotDBImpl(); // 构造函数(自动Init)
    PlotDBImpl(const PlotDBImpl&)            = delete;
    PlotDBImpl& operator=(const PlotDBImpl&) = delete;

    // ============ Table CRUD API ============


    // PlotAdmins
    PLAPI bool hasAdmin(UUID const& uuid);
    PLAPI bool isAdmin(UUID const& uuid);
    PLAPI bool addAdmin(UUID const& uuid);
    PLAPI bool removeAdmin(UUID const& uuid);
    PLAPI std::vector<UUID> getAdmins();

    // Plots
    PLAPI bool hasPlot(PlotID const& pid);
    PLAPI bool isPlotOwner(PlotID const& pid, UUID const& uid);
    PLAPI bool addPlot(PlotPos& pos, UUID const& uid, string const& name);
    PLAPI bool removePlot(PlotID const& pid);
    PLAPI bool updatePlotOwner(PlotID const& pid, UUID const& newOwner);
    PLAPI bool updatePlotName(PlotID const& pid, string const& newName);

    PLAPI std::optional<Plot> getPlot(PlotID const& id);

    PLAPI Plots getPlots(UUID const& uid);
    PLAPI Plots getPlots();


    // PlotShares
    PLAPI bool hasPlotShared(PlotID const& id);
    PLAPI bool isPlotSharedPlayer(PlotID const& id, UUID const& uid);
    PLAPI bool addShareInfo(PlotID const& id, UUID const& targetPlayer);
    PLAPI bool removeSharedInfo(PlotID const& id, UUID const& uid);
    PLAPI bool resetPlotShareInfo(PlotID const& id); // 重置共享信息

    PLAPI std::optional<PlotShare> getSharedPlot(PlotID const& id, UUID const& uid);

    PLAPI PlotShares getSharedPlots(PlotID const& id);
    PLAPI PlotShares getSharedPlots(UUID const& uid);
    PLAPI PlotShares getSharedPlots();


    // PlotCommenets
    PLAPI bool hasComment(CommentID const& cid);
    PLAPI bool isCommentPlayer(CommentID const& cid, UUID const& uid); // 判断某个评论是否是某个玩家的
    PLAPI bool addComment(PlotID const& pid, UUID const& uid, string const& content);
    PLAPI bool removeComment(CommentID const& cid);                        // 删除某个评论
    PLAPI bool removeComments(PlotID const& pid);                          // 删除某个地皮的所有评论
    PLAPI bool removeComments(UUID const& uid);                            // 删除某个玩家的所有评论
    PLAPI bool updateComment(CommentID const& cid, string const& content); // 更新某个评论的内容

    PLAPI std::optional<PlotComment> getComment(CommentID const& cid);

    PLAPI PlotCommenets getComments(PlotID const& pid, UUID const& uid);
    PLAPI PlotCommenets getComments(PlotID const& pid);
    PLAPI PlotCommenets getComments(UUID const& uid);
    PLAPI PlotCommenets getComments();


    // PlotSales
    PLAPI bool hasSale(PlotID const& pid);
    PLAPI bool addSale(PlotID const& pid, int price);
    PLAPI bool removeSale(PlotID const& pid);
    PLAPI bool updateSale(PlotID const& pid, int price);

    PLAPI std::optional<PlotSale> getSale(PlotID const& pid);
    PLAPI PlotSales               getSales();

    // 从出售中购买地皮
    PLAPI bool buyPlotFromSale(PlotID const& pid, UUID const& buyer, bool resetShares = true);

    // 获取玩家权限等级
    PLAPI PlotPermission getPlayerPermission(UUID const& uid, PlotID const& pid, bool ignoreAdmin = false);
};


// Class: PlotDB
using DynamicCache = std::variant<Plot, PlotShare, bool>;
class PlotDB {
private:
    std::unordered_map<size_t, Plot>      mPlots;      // 缓存地皮信息 key: hash(PlotID)
    std::unordered_map<size_t, PlotShare> mPlotShares; // 缓存共享信息 key: hash(PlotID + SharedPlayer(UUID))
    std::unordered_map<UUID, bool>        mAdmins;     // 缓存管理员信息 key: UUID

    std::unique_ptr<PlotDBImpl> mImpl; // 数据库实例

    PlotDB()                         = default;
    PlotDB(const PlotDB&)            = delete;
    PlotDB& operator=(const PlotDB&) = delete;

public:
    enum class CacheType {
        All,
        Plot,
        PlotShare,
        Admin,
    };

    PLAPI PlotDBImpl&    getImpl();
    PLAPI static PlotDB& getInstance();

    PLAPI bool load();
    PLAPI bool initCache(CacheType type = CacheType::All);
    PLAPI bool resetCache(CacheType type = CacheType::All);

    PLAPI size_t hash(PlotID const& pid);                  // Plot
    PLAPI size_t hash(PlotID const& pid, UUID const& uid); // PlotShare

    PLAPI bool cache(Plot const& plot);                                            // Plot
    PLAPI bool cache(PlotShare const& share);                                      // PlotShare
    PLAPI bool cache(UUID const& uuid);                                            // PlotAdmin
    PLAPI bool cache(size_t const& key, CacheType type, DynamicCache const& data); // Custom

    PLAPI bool hasCached(PlotID const& pid);                  // Plot
    PLAPI bool hasCached(PlotID const& pid, UUID const& uid); // PlotShare
    PLAPI bool hasCached(UUID const& uuid);                   // PlotAdmin
    PLAPI bool hasCached(size_t const& key, CacheType type);  // Custom

    PLAPI std::optional<Plot> getCached(PlotID const& id);                          // Plot
    PLAPI std::optional<PlotShare> getCached(PlotID const& id, UUID const& uid);    // PlotShare
    PLAPI std::optional<bool> getCached(UUID const& uuid);                          // PlotAdmin
    PLAPI std::optional<DynamicCache> getCached(size_t const& key, CacheType type); // Custom

    PLAPI bool removeCached(PlotID const& pid);                  // Plot
    PLAPI bool removeCached(PlotID const& pid, UUID const& uid); // PlotShare
    PLAPI bool removeCached(UUID const& uuid);                   // PlotAdmin
    PLAPI bool removeCached(size_t const& key, CacheType type);  // Custom


    PLAPI bool updateCachedPlotName(PlotID const& pid, string const& newName); // Plot
    PLAPI bool updateCachedPlotOwner(PlotID const& pid, UUID const& newOwner); // Plot
    PLAPI PlotPermission
    getPermission(UUID const& uuid, PlotID const& pid, bool ignoreAdmin = false, bool ignoreCache = false);
};

} // namespace plo::database
