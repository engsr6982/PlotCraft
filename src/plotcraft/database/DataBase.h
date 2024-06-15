#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteCpp/Statement.h"
#include "ll/api/data/KeyValueDB.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/core/PlotPos.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <optional>
#include <string>


using string = std::string;
using namespace plotcraft::core;

namespace plotcraft::database {


// typedefs
typedef string    PlotID; // PlotPos::toString()
typedef mce::UUID UUID;


class PlayerNameDB {
private:
    std::unique_ptr<ll::data::KeyValueDB> mPlayerNameDB;
    bool                                  isInit = false;

    PlayerNameDB()                               = default;
    PlayerNameDB(const PlayerNameDB&)            = delete;
    PlayerNameDB& operator=(const PlayerNameDB&) = delete;

public:
    static PlayerNameDB& getInstance();
    bool                 initPlayerNameDB();

    bool hasPlayer(string const& realName);
    bool hasPlayer(mce::UUID const& uuid);

    std::optional<string>    getPlayerName(mce::UUID const& uuid);
    std::optional<mce::UUID> getPlayerUUID(string const& realName);

    bool insertPlayer(Player& player);
};


// Table Structure
enum class PlotPermission : int {
    UnKnown = 0, // 未知权限
    Shared  = 1, // 共享者
    Owner   = 2, // 主人
    Admin   = 3  // 管理员
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


#define HandleSQLiteExceptionAndReturn(defv)                                                                           \
    catch (SQLite::Exception const& e) {                                                                               \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "Fail in {}, SQLite::Exception: {}",                                                                       \
            __FUNCTION__,                                                                                              \
            e.what()                                                                                                   \
        );                                                                                                             \
        return defv;                                                                                                   \
    }
#define HandleSQLiteException()                                                                                        \
    catch (SQLite::Exception const& e) {                                                                               \
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(                                                \
            "Fail in {}, SQLite::Exception: {}",                                                                       \
            __FUNCTION__,                                                                                              \
            e.what()                                                                                                   \
        );                                                                                                             \
    }


// Class: SQLite
class PlotDB {
private:
    std::unique_ptr<SQLite::Database> mSQLite;
    bool                              isInit = false;

    PlotDB()                         = default;
    PlotDB(const PlotDB&)            = delete;
    PlotDB& operator=(const PlotDB&) = delete;

    void initTables();

public:
    static PlotDB& getInstance();

    bool initSQLite();

    // ============ Table CRUD API ============


    // PlotAdmins
    bool hasAdmin(UUID const& uuid);
    bool isAdmin(UUID const& uuid);
    bool addAdmin(UUID const& uuid);
    bool removeAdmin(UUID const& uuid);


    // Plots
    bool hasPlot(PlotID const& pid);
    bool isPlotOwner(UUID const& uid);
    bool addPlot(PlotPos& pos, UUID const& uid, string const& name);
    bool removePlot(PlotID const& pid);
    bool updatePlotOwner(PlotID const& pid, UUID const& newOwner);
    bool updatePlotName(PlotID const& pid, string const& newName);

    std::optional<Plot> getPlot(PlotID const& id);

    Plots getPlots(UUID const& uid);
    Plots getPlots();


    // PlotShares
    bool hasPlotShared(PlotID const& id);
    bool isPlotSharedPlayer(PlotID const& id, UUID const& uid);
    bool addShareInfo(PlotID const& id, UUID const& targetPlayer);
    bool removeSharedInfo(PlotID const& id, UUID const& uid);
    bool resetPlotShareInfo(PlotID const& id); // 重置共享信息

    std::optional<PlotShare> getSharedPlot(PlotID const& id, UUID const& uid);

    PlotShares getSharedPlots(PlotID const& id);
    PlotShares getSharedPlots(UUID const& uid);
    PlotShares getSharedPlots();


    // PlotCommenets
    bool hasComment(CommentID const& cid);
    bool isCommentPlayer(CommentID const& cid, UUID const& uid); // 判断某个评论是否是某个玩家的
    bool addComment(PlotID const& pid, UUID const& uid, string const& content);
    bool removeComment(CommentID const& cid);                        // 删除某个评论
    bool removeComments(PlotID const& pid);                          // 删除某个地皮的所有评论
    bool removeComments(UUID const& uid);                            // 删除某个玩家的所有评论
    bool updateComment(CommentID const& cid, string const& content); // 更新某个评论的内容

    std::optional<PlotComment> getComment(CommentID const& cid);

    PlotCommenets getComments(PlotID const& pid, UUID const& uid);
    PlotCommenets getComments(PlotID const& pid);
    PlotCommenets getComments(UUID const& uid);
    PlotCommenets getComments();


    // PlotSales
    bool hasSale(PlotID const& pid);
    bool addSale(PlotID const& pid, int price);
    bool removeSale(PlotID const& pid);
    bool updateSale(PlotID const& pid, int price);

    std::optional<PlotSale> getSale(PlotID const& pid);
    PlotSales               getSales();

    // 从出售中购买地皮
    bool buyPlotFromSale(PlotID const& pid, UUID const& buyer, bool resetShares = true);
};

} // namespace plotcraft::database