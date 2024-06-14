#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "ll/api/data/KeyValueDB.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <string>


using string = std::string;


namespace plotcraft::database {

class PlayerNameDB {
private:
    std::unique_ptr<ll::data::KeyValueDB> mPlayerNameDB;
    bool                                  isInit = false;

    PlayerNameDB()                               = default;
    PlayerNameDB(const PlayerNameDB&)            = delete;
    PlayerNameDB& operator=(const PlayerNameDB&) = delete;

public:
    static PlayerNameDB& getInstance();

    bool initPlayerNameDB();

    bool hasPlayer(const string& realName);
    bool hasPlayer(const mce::UUID& uuid);

    string    getPlayerName(const mce::UUID& uuid);
    mce::UUID getPlayerUUID(const string& realName);

    bool insertPlayer(Player& player);
};


// Table Structure
enum class PlotPermission : int {
	UnKnown = 0, // 未知权限
	Shared  = 1, // 共享者
	Owner   = 2, // 主人
	Admin   = 3  // 管理员
};

enum class PlotSaleStatus : int {
	UnKnown  = -1, // 未知
	Saleing  = 0,  // 出售中
	Saled    = 1,  // 已出售
	Canceled = 2   // 已取消
};


// typedef

typedef string    PlotID;
typedef mce::UUID UUID;


struct PlotAdmin {
	UUID mUUID; // 主键
};
using PlotAdmins = std::vector<PlotAdmin>;

struct Plot {
	PlotID mPlotID;    // 主键
	string mPlotName;
	UUID   mPlotOwner; // 主键
	int    mPlotX;
	int    mPlotZ;
};
using Plots = std::vector<Plot>;

struct PlotShare {
	PlotID  mPlotID;       // 主键
	UUID    mSharedPlayer; // 主键
	string  mSharedTime;
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
	PlotID mPlotID;       // 主键
	int    mPrice;
	string mSaleTime;
	UUID   mSellerPlayer; // 主键
	UUID   mBuyerPlayer;  // 次键
	string mBuyTime;
	PlotSaleStatus mStatus;
};
using PlotSales = std::vector<PlotSale>;



// Class: SQLite
class SQLite {
private:
    std::unique_ptr<::SQLite::Database> mSQLite;
    bool                                isInit = false;

    SQLite()                         = default;
    SQLite(const SQLite&)            = delete;
    SQLite& operator=(const SQLite&) = delete;

public:
    static SQLite& getInstance();

    bool initSQLite();

    ////////// Table CRUD API //////////

    // PlotAdmins
    bool hasAdmin(UUID const& uuid);
    bool isAdmin(UUID const& uuid);
    bool addAdmin(UUID const& uuid);
    bool removeAdmin(UUID const& uuid);
    
    // Plots
    bool hasPlot(PlotID const& pid);
    bool isPlotOwner(UUID const& uid);
    
    bool addPlot(PlotPos const& pos, UUID const& uid, string const& name = "");
    
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

    std::optional<PlotShare> getSharedPlot(PlotID const& id, UUID const& uid);

    PlotShares getSharedPlots(PlotID const& id);
    PlotShares getSharedPlots(UUID const& uid);
    PlotShares getSharedPlots();

    // PlotCommenets
    bool hasComment(CommentID const& cid);
    bool isCommentPlayer(CommentID const& cid, UUID const& uid);
    
    // PlotSales
};

} // namespace plotcraft::database
