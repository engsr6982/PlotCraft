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


/*

## SQLite 表结构

### PlotGlobalAdmins

- uuid: 管理员的 UUID

### PlotPermissions

- permission_id: 本条权限的 ID
- plot_id: 地皮 ID
- destory_block: 是否允许破坏方块
- place_block: 是否允许放置方块
- ...: 待补充

### Plots

- plot_id: 地皮 ID
- owner: 地皮所有者的 UUID
- plot_x: 地皮 X 坐标
- plot_z: 地皮 Z 坐标
- plot_name: 地皮名称

### PlotShares

- share_id: 共享 ID
- plot_id: 共享的地皮 ID
- player_uuid: 共享者的 UUID
- shared_time: 共享时间
- bind_permission: 共享地皮的权限

### PlotCommenets

- comment_id: 评论 ID
- plot_id: 评论的地皮 ID
- player_uuid: 评论者的 UUID
- comment_time: 评论时间
- content: 评论内容

### PlotSales

- sale_id: 出售 ID
- plot_id: 出售的地皮 ID
- seller_uuid: 出售者的 UUID
- buyer_uuid: 购买者的 UUID
- sale_time: 出售时间
- buy_time: 购买时间
- price: 出售价格
- status: 出售状态（待出售/已出售/已取消）

### PlotTransfers

- transfer_id: 转让 ID
- plot_id: 转让的地皮 ID
- old_owner: 原所有者的 UUID
- new_owner: 新所有者的 UUID
- transfer_time: 转让时间

### PlotMerges

- merge_id: 合并 ID
- merged_player: 执行合并的玩家 UUID
- merged_plots: 被合并的地皮 ID 列表
- merge_time: 合并时间

 */

// Global Vars
enum class QueryTable : int {
    Unknown          = 0,
    PlotGlobalAdmins = 1, // 全局管理员
    PlotPermissions  = 2, // 地皮权限
    Plots            = 3, // 地皮信息
    PlotShares       = 4, // 地皮共享
    PlotComments     = 5, // 地皮评论
    PlotSales        = 6, // 地皮出售
    PlotTransfers    = 7, // 地皮转让
    PlotMerges       = 8  // 地皮合并
};

typedef int    PermissionID;
typedef string PlotID;
typedef int    ShareID;
typedef int    CommentID;
typedef int    SaleID;
typedef int    TransferID;
typedef int    MergeID;

// Table structs

enum class PlotSaleStatus : int {
    UnKnown  = -1, // 未知状态
    Pending  = 0,  // 待出售
    Sold     = 1,  // 已出售
    Canceled = 2   // 已取消
};


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