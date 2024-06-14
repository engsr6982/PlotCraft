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

typedef string PlotID;
typedef int    ShareID;
typedef int    CommentID;
typedef int    SaleID;


struct PlotAdmin {
	string mUUID;
};
using PlotAdmins = std::vector<PlotAdmin>;

struct Plot {
	PlotID mPlotID;
	string mPlotName;
	string mPlotOwner;
	int    mPlotX;
	int    mPlotZ;
};
using Plots = std::vector<Plot>;

struct PlotShare {
	ShareID mShareID;
	PlotID  mPlotID;
	string  mSharedPlayer;
	string  mSharedTime;
};
using PlotShares = std::vector<PlotShare>;

struct PlotComment {
	CommentID mCommentID;
	PlotID    mPlotID;
	string    mCommentPlater;
	string    mCommentTime;
	string    mContent;
};
using PlotCommenets = std::vector<PlotComment>;
	
struct PlotSale {
	SaleID mSaleID;
	PlotID mPlotID;
	int    mPrice;
	string mSaleTime;
	string mSellerUUID;
	string mBuyerUUID;
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

    bool hasTable(const string& tableName);


    // 全局管理员
    bool hasGlobalAdmin(const mce::UUID& uuid);
    bool addGlobalAdmin(const mce::UUID& uuid);
    bool removeGlobalAdmin(const mce::UUID& uuid);


    // 地皮权限表
    bool hasPermissionInfo(const int& permissionId); // 检查是否有此条权限信息
    bool hasPermissionInfo(const string& plotId);
    bool hasPermission(const int& permissionId, const PermissionType& permissionType); // 检查是否有权限
    bool hasPermission(const string& plotId, const PermissionType& permissionType);

    bool createPermissionInfo(const string& plotId);    // 创建权限信息
    bool removePermissionInfo(const int& permissionId); // 删除权限信息
    bool removePermissionInfo(const string& plotId);    // 删除权限信息(与地皮ID匹配所有权限记录)
    bool updatePermission(const int& permissionId, const PermissionType& permissionType, const bool& value); // 更新权限
};

} // namespace plotcraft::database
