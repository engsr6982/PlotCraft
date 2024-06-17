#include "DataBase.h"
#include "fmt/color.h"
#include "plotcraft/utils/Date.h"
#include <cstddef>
#include <stdexcept>


namespace plo::database {

// ======================= Class PlayerNameDB =======================
PlayerNameDB& PlayerNameDB::getInstance() {
    static PlayerNameDB instance;
    return instance;
}
bool PlayerNameDB::initPlayerNameDB() {
    if (isInit) return true;
    mPlayerNameDB = std::make_unique<ll::data::KeyValueDB>(
        my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlayerNameDB"
    );
    return isInit = true;
}

bool PlayerNameDB::hasPlayer(string const& realName) { return mPlayerNameDB->has(realName); }
bool PlayerNameDB::hasPlayer(mce::UUID const& uuid) { return mPlayerNameDB->has(uuid.asString()); }

std::optional<string> PlayerNameDB::getPlayerName(mce::UUID const& uuid) { return mPlayerNameDB->get(uuid.asString()); }
std::optional<mce::UUID> PlayerNameDB::getPlayerUUID(string const& realName) {
    if (hasPlayer(realName)) return mce::UUID::fromString(*mPlayerNameDB->get(realName));
    return std::nullopt;
}

bool PlayerNameDB::insertPlayer(Player& player) {
    if (hasPlayer(player.getUuid()) || hasPlayer(player.getRealName())) return false;
    string uuidStr = player.getUuid().asString();
    string name    = player.getRealName();
    mPlayerNameDB->set(uuidStr, name);
    mPlayerNameDB->set(name, uuidStr);
    return true;
}


// ======================= Class PlotDBImpl =======================
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

// private
void PlotDBImpl::initTables() {
    try {
        // Table: PlotAdmins
        mSQLite->exec(R"(
                CREATE TABLE IF NOT EXISTS "PlotAdmins" (
                    "mUUID"	TEXT NOT NULL UNIQUE,
            	    PRIMARY KEY("mUUID")
                );
            )");
        // Table: Plots
        mSQLite->exec(R"(
                CREATE TABLE IF NOT EXISTS "Plots" (
                    "mPlotID"	TEXT NOT NULL UNIQUE,
            	    "mPlotName"	TEXT,
            	    "mPlotOwner"	TEXT NOT NULL,
            	    "mPlotX"	INTEGER NOT NULL,
            	    "mPlotZ"	INTEGER NOT NULL,
                    PRIMARY KEY("mPlotID","mPlotOwner")
                );
            )");
        // Table: PlotShares
        mSQLite->exec(R"(
                CREATE TABLE IF NOT EXISTS "PlotShares" (
            	    "mPlotID"	TEXT NOT NULL,
            	    "mSharedPlayer"	TEXT NOT NULL,
            	    "mSharedTime"	TEXT DEFAULT CURRENT_TIMESTAMP,
            	    PRIMARY KEY("mPlotID","mSharedPlayer")
                );
            )");
        // Table: PlotCommenets
        mSQLite->exec(R"(
                CREATE TABLE IF NOT EXISTS "PlotCommenets" (
            	    "mCommentID"	INTEGER NOT NULL UNIQUE,
            	    "mPlotID"	TEXT NOT NULL,
            	    "mCommentPlayer"	TEXT NOT NULL,
            	    "mCommentTime"	TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            	    "mContent"	TEXT NOT NULL,
            	    PRIMARY KEY("mCommentID" AUTOINCREMENT)
                );
            )");
        // Table: PlotSales
        mSQLite->exec(R"(
                CREATE TABLE IF NOT EXISTS "PlotSales" (
            	    "mPlotID"	TEXT NOT NULL UNIQUE,
            	    "mPrice"	INTEGER,
            	    "mSaleTime"	TEXT DEFAULT CURRENT_TIMESTAMP,
            	    PRIMARY KEY("mPlotID")
                );
            )");
    }
    HandleSQLiteException();
}
bool PlotDBImpl::initSQLite() {
    if (isInit) return true;
    // Initialize SQLite
    mSQLite = std::make_unique<SQLite::Database>(
        my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlotCraft.db",
        SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE
    );
    initTables();
    return isInit = true;
}
// public
PlotDBImpl::PlotDBImpl() {
    initSQLite();
    initTables();
}

// ======================= PlotAdmins APIs =======================
bool PlotDBImpl::hasAdmin(UUID const& uuid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotAdmins WHERE mUUID = ?");
    query.bind(1, uuid.asString());
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}

bool PlotDBImpl::isAdmin(UUID const& uuid) { return hasAdmin(uuid); }

bool PlotDBImpl::addAdmin(UUID const& uuid) {
    try {
        SQLite::Statement query(*mSQLite, "INSERT INTO PlotAdmins (mUUID) VALUES (?)");
        query.bind(1, uuid.asString());
        query.exec();
        PlotDB::getInstance().cache(uuid); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::removeAdmin(UUID const& uuid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotAdmins WHERE mUUID = ?");
        query.bind(1, uuid.asString());
        query.exec();
        PlotDB::getInstance().removeCached(uuid); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
std::vector<UUID> PlotDBImpl::getAdmins() {
    std::vector<UUID> admins;
    SQLite::Statement query(*mSQLite, "SELECT mUUID FROM PlotAdmins");
    while (query.executeStep()) {
        admins.push_back(UUID::fromString(query.getColumn(0).getText()));
    }
    return admins;
}


// ======================= Plots APIs =======================
bool PlotDBImpl::hasPlot(PlotID const& pid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM Plots WHERE mPlotID = ?");
    query.bind(1, pid);
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}
bool PlotDBImpl::isPlotOwner(PlotID const& pid, UUID const& uid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM Plots WHERE mPlotOwner = ? AND mPlotID = ?");
    query.bind(1, uid.asString());
    query.bind(2, pid);
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}
bool PlotDBImpl::addPlot(PlotPos& pos, UUID const& uid, string const& name) {
    try {
        SQLite::Statement query(
            *mSQLite,
            "INSERT INTO Plots (mPlotID, mPlotName, mPlotOwner, mPlotX, mPlotZ) VALUES (?, ?, ?, ?, ?)"
        );
        string plotID = pos.toString();
        query.bind(1, plotID);
        query.bind(2, name);
        query.bind(3, uid.asString());
        query.bind(4, pos.x);
        query.bind(5, pos.z);
        query.exec();
        PlotDB::getInstance().cache(Plot{pos.toString(), name, uid, pos.x, pos.z}); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
bool PlotDBImpl::removePlot(PlotID const& pid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM Plots WHERE mPlotID = ?");
        query.bind(1, pid);
        query.exec();
        PlotDB::getInstance().removeCached(pid); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
bool PlotDBImpl::updatePlotOwner(PlotID const& pid, UUID const& newOwner) {
    try {
        SQLite::Statement query(*mSQLite, "UPDATE Plots SET mPlotOwner = ? WHERE mPlotID = ?");
        query.bind(1, newOwner.asString());
        query.bind(2, pid);
        query.exec();
        PlotDB::getInstance().updateCachedPlotOwner(pid, newOwner); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::updatePlotName(PlotID const& pid, string const& newName) {
    try {
        SQLite::Statement query(*mSQLite, "UPDATE Plots SET mPlotName = ? WHERE mPlotID = ?");
        query.bind(1, newName);
        query.bind(2, pid);
        query.exec();
        PlotDB::getInstance().updateCachedPlotName(pid, newName); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

std::optional<Plot> PlotDBImpl::getPlot(PlotID const& id) {
    SQLite::Statement query(*mSQLite, "SELECT * FROM Plots WHERE mPlotID = ?");
    query.bind(1, id);
    if (query.executeStep()) {
        Plot plot;
        plot.mPlotID    = query.getColumn("mPlotID").getText();
        plot.mPlotName  = query.getColumn("mPlotName").getText();
        plot.mPlotOwner = UUID::fromString(query.getColumn("mPlotOwner").getText());
        plot.mPlotX     = query.getColumn("mPlotX").getInt();
        plot.mPlotZ     = query.getColumn("mPlotZ").getInt();
        PlotDB::getInstance().cache(plot); // cache
        return plot;
    }
    return std::nullopt;
}

Plots PlotDBImpl::getPlots(UUID const& uid) {
    Plots             plots;
    SQLite::Statement query(*mSQLite, "SELECT * FROM Plots WHERE mPlotOwner = ?");
    query.bind(1, uid.asString());
    while (query.executeStep()) {
        Plot plot;
        plot.mPlotID    = query.getColumn("mPlotID").getText();
        plot.mPlotName  = query.getColumn("mPlotName").getText();
        plot.mPlotOwner = UUID::fromString(query.getColumn("mPlotOwner").getText());
        plot.mPlotX     = query.getColumn("mPlotX").getInt();
        plot.mPlotZ     = query.getColumn("mPlotZ").getInt();
        PlotDB::getInstance().cache(plot); // cache
        plots.push_back(plot);
    }
    return plots;
}

Plots PlotDBImpl::getPlots() {
    Plots             plots;
    SQLite::Statement query(*mSQLite, "SELECT * FROM Plots");
    while (query.executeStep()) {
        Plot plot;
        plot.mPlotID    = query.getColumn("mPlotID").getText();
        plot.mPlotName  = query.getColumn("mPlotName").getText();
        plot.mPlotOwner = UUID::fromString(query.getColumn("mPlotOwner").getText());
        plot.mPlotX     = query.getColumn("mPlotX").getInt();
        plot.mPlotZ     = query.getColumn("mPlotZ").getInt();
        PlotDB::getInstance().cache(plot); // cache
        plots.push_back(plot);
    }
    return plots;
}


// ======================= PlotShares APIs =======================

bool PlotDBImpl::hasPlotShared(PlotID const& id) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotShares WHERE mPlotID = ?");
    query.bind(1, id);
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}
bool PlotDBImpl::isPlotSharedPlayer(PlotID const& id, UUID const& uid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotShares WHERE mPlotID = ? AND mSharedPlayer = ?");
    query.bind(1, id);
    query.bind(2, uid.asString());
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}
bool PlotDBImpl::addShareInfo(PlotID const& id, UUID const& targetPlayer) {
    try {
        SQLite::Statement query(*mSQLite, "INSERT INTO PlotShares (mPlotID, mSharedPlayer) VALUES (?, ?)");
        query.bind(1, id);
        query.bind(2, targetPlayer.asString());
        query.exec();
        PlotDB::getInstance().cache(PlotShare{id, targetPlayer, plo::utils::Date{}.toString()}); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
bool PlotDBImpl::removeSharedInfo(PlotID const& id, UUID const& uid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotShares WHERE mPlotID = ? AND mSharedPlayer = ?");
        query.bind(1, id);
        query.bind(2, uid.asString());
        query.exec();
        PlotDB::getInstance().removeCached(id, uid); // cache
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
bool PlotDBImpl::resetPlotShareInfo(PlotID const& id) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotShares WHERE mPlotID = ?");
        query.bind(1, id);
        query.exec();
        PlotDB::getInstance().resetCache(PlotDB::CacheType::PlotShare); // cache (因为key是player + id，无法精确匹配)
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}
std::optional<PlotShare> PlotDBImpl::getSharedPlot(PlotID const& id, UUID const& uid) {
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotShares WHERE mPlotID = ? AND mSharedPlayer = ?");
    query.bind(1, id);
    query.bind(2, uid.asString());
    if (query.executeStep()) {
        PlotShare share;
        share.mPlotID       = query.getColumn("mPlotID").getText();
        share.mSharedPlayer = UUID::fromString(query.getColumn("mSharedPlayer").getText());
        share.mSharedTime   = query.getColumn("mSharedTime").getText();
        PlotDB::getInstance().cache(share); // cache
        return share;
    }
    return std::nullopt;
}
PlotShares PlotDBImpl::getSharedPlots(PlotID const& id) {
    PlotShares        shares;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotShares WHERE mPlotID = ?");
    query.bind(1, id);
    while (query.executeStep()) {
        PlotShare share;
        share.mPlotID       = query.getColumn("mPlotID").getText();
        share.mSharedPlayer = UUID::fromString(query.getColumn("mSharedPlayer").getText());
        share.mSharedTime   = query.getColumn("mSharedTime").getText();
        PlotDB::getInstance().cache(share); // cache
        shares.push_back(share);
    }
    return shares;
}
PlotShares PlotDBImpl::getSharedPlots(UUID const& uid) {
    PlotShares        shares;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotShares WHERE mSharedPlayer = ?");
    query.bind(1, uid.asString());
    while (query.executeStep()) {
        PlotShare share;
        share.mPlotID       = query.getColumn("mPlotID").getText();
        share.mSharedPlayer = UUID::fromString(query.getColumn("mSharedPlayer").getText());
        share.mSharedTime   = query.getColumn("mSharedTime").getText();
        PlotDB::getInstance().cache(share); // cache
        shares.push_back(share);
    }
    return shares;
}

PlotShares PlotDBImpl::getSharedPlots() {
    PlotShares        shares;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotShares");
    while (query.executeStep()) {
        PlotShare share;
        share.mPlotID       = query.getColumn("mPlotID").getText();
        share.mSharedPlayer = UUID::fromString(query.getColumn("mSharedPlayer").getText());
        share.mSharedTime   = query.getColumn("mSharedTime").getText();
        PlotDB::getInstance().cache(share); // cache
        shares.push_back(share);
    }
    return shares;
}


// ======================= PlotCommenets APIs =======================
bool PlotDBImpl::hasComment(CommentID const& cid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotCommenets WHERE mCommentID = ?");
    query.bind(1, cid);
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}

bool PlotDBImpl::isCommentPlayer(CommentID const& cid, UUID const& uid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotCommenets WHERE mCommentID = ? AND mCommentPlayer = ?");
    query.bind(1, cid);
    query.bind(2, uid.asString());
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}

bool PlotDBImpl::addComment(PlotID const& pid, UUID const& uid, string const& content) {
    try {
        SQLite::Statement query(
            *mSQLite,
            "INSERT INTO PlotCommenets (mPlotID, mCommentPlayer, mContent) VALUES (?, ?, ?)"
        );
        query.bind(1, pid);
        query.bind(2, uid.asString());
        query.bind(3, content);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::removeComment(CommentID const& cid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotCommenets WHERE mCommentID = ?");
        query.bind(1, cid);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::removeComments(PlotID const& pid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotCommenets WHERE mPlotID = ?");
        query.bind(1, pid);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::removeComments(UUID const& uid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotCommenets WHERE mCommentPlayer = ?");
        query.bind(1, uid.asString());
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::updateComment(CommentID const& cid, string const& content) {
    try {
        SQLite::Statement query(*mSQLite, "UPDATE PlotCommenets SET mContent = ? WHERE mCommentID = ?");
        query.bind(1, content);
        query.bind(2, cid);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

std::optional<PlotComment> PlotDBImpl::getComment(CommentID const& cid) {
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotCommenets WHERE mCommentID = ?");
    query.bind(1, cid);
    if (query.executeStep()) {
        PlotComment comment;
        comment.mCommentID     = query.getColumn("mCommentID").getInt();
        comment.mPlotID        = query.getColumn("mPlotID").getText();
        comment.mCommentPlayer = UUID::fromString(query.getColumn("mCommentPlayer").getText());
        comment.mCommentTime   = query.getColumn("mCommentTime").getText();
        comment.mContent       = query.getColumn("mContent").getText();
        return comment;
    }
    return std::nullopt;
}

PlotCommenets PlotDBImpl::getComments(PlotID const& pid, UUID const& uid) {
    PlotCommenets     comments;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotCommenets WHERE mPlotID = ? AND mCommentPlayer = ?");
    query.bind(1, pid);
    query.bind(2, uid.asString());
    while (query.executeStep()) {
        PlotComment comment;
        comment.mCommentID     = query.getColumn("mCommentID").getInt();
        comment.mPlotID        = query.getColumn("mPlotID").getText();
        comment.mCommentPlayer = UUID::fromString(query.getColumn("mCommentPlayer").getText());
        comment.mCommentTime   = query.getColumn("mCommentTime").getText();
        comment.mContent       = query.getColumn("mContent").getText();
        comments.push_back(comment);
    }
    return comments;
}

PlotCommenets PlotDBImpl::getComments(PlotID const& pid) {
    PlotCommenets     comments;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotCommenets WHERE mPlotID = ?");
    query.bind(1, pid);
    while (query.executeStep()) {
        PlotComment comment;
        comment.mCommentID     = query.getColumn("mCommentID").getInt();
        comment.mPlotID        = query.getColumn("mPlotID").getText();
        comment.mCommentPlayer = UUID::fromString(query.getColumn("mCommentPlayer").getText());
        comment.mCommentTime   = query.getColumn("mCommentTime").getText();
        comment.mContent       = query.getColumn("mContent").getText();
        comments.push_back(comment);
    }
    return comments;
}

PlotCommenets PlotDBImpl::getComments(UUID const& uid) {
    PlotCommenets     comments;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotCommenets WHERE mCommentPlayer = ?");
    query.bind(1, uid.asString());
    while (query.executeStep()) {
        PlotComment comment;
        comment.mCommentID     = query.getColumn("mCommentID").getInt();
        comment.mPlotID        = query.getColumn("mPlotID").getText();
        comment.mCommentPlayer = UUID::fromString(query.getColumn("mCommentPlayer").getText());
        comment.mCommentTime   = query.getColumn("mCommentTime").getText();
        comment.mContent       = query.getColumn("mContent").getText();
        comments.push_back(comment);
    }
    return comments;
}

PlotCommenets PlotDBImpl::getComments() {
    PlotCommenets     comments;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotCommenets");
    while (query.executeStep()) {
        PlotComment comment;
        comment.mCommentID     = query.getColumn("mCommentID").getInt();
        comment.mPlotID        = query.getColumn("mPlotID").getText();
        comment.mCommentPlayer = UUID::fromString(query.getColumn("mCommentPlayer").getText());
        comment.mCommentTime   = query.getColumn("mCommentTime").getText();
        comment.mContent       = query.getColumn("mContent").getText();
        comments.push_back(comment);
    }
    return comments;
}


// ======================= PlotSales APIs =======================

bool PlotDBImpl::hasSale(PlotID const& pid) {
    SQLite::Statement query(*mSQLite, "SELECT COUNT(*) FROM PlotSales WHERE mPlotID = ?");
    query.bind(1, pid);
    if (query.executeStep()) {
        return query.getColumn(0).getInt() > 0;
    }
    return false;
}

bool PlotDBImpl::addSale(PlotID const& pid, int price) {
    try {
        SQLite::Statement query(*mSQLite, "INSERT INTO PlotSales (mPlotID, mPrice) VALUES (?, ?)");
        query.bind(1, pid);
        query.bind(2, price);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::removeSale(PlotID const& pid) {
    try {
        SQLite::Statement query(*mSQLite, "DELETE FROM PlotSales WHERE mPlotID = ?");
        query.bind(1, pid);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

bool PlotDBImpl::updateSale(PlotID const& pid, int price) {
    try {
        SQLite::Statement query(*mSQLite, "UPDATE PlotSales SET mPrice = ? WHERE mPlotID = ?");
        query.bind(1, price);
        query.bind(2, pid);
        query.exec();
        return true;
    }
    HandleSQLiteExceptionAndReturn(false);
}

std::optional<PlotSale> PlotDBImpl::getSale(PlotID const& pid) {
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotSales WHERE mPlotID = ?");
    query.bind(1, pid);
    if (query.executeStep()) {
        PlotSale sale;
        sale.mPlotID   = query.getColumn("mPlotID").getText();
        sale.mPrice    = query.getColumn("mPrice").getInt();
        sale.mSaleTime = query.getColumn("mSaleTime").getText();
        return sale;
    }
    return std::nullopt;
}

PlotSales PlotDBImpl::getSales() {
    PlotSales         sales;
    SQLite::Statement query(*mSQLite, "SELECT * FROM PlotSales");
    while (query.executeStep()) {
        PlotSale sale;
        sale.mPlotID   = query.getColumn("mPlotID").getText();
        sale.mPrice    = query.getColumn("mPrice").getInt();
        sale.mSaleTime = query.getColumn("mSaleTime").getText();
        sales.push_back(sale);
    }
    return sales;
}

// 购买地皮
bool PlotDBImpl::buyPlotFromSale(PlotID const& pid, UUID const& buyer, bool resetShares) {
    try {
        // Begin transaction
        mSQLite->exec("BEGIN");

        // 更新地皮所有者
        SQLite::Statement updateOwner(*mSQLite, "UPDATE Plots SET mPlotOwner = ? WHERE mPlotID = ?");
        updateOwner.bind(1, buyer.asString());
        updateOwner.bind(2, pid);
        updateOwner.exec();

        // 移除本条出售信息
        SQLite::Statement removeSale(*mSQLite, "DELETE FROM PlotSales WHERE mPlotID = ?");
        removeSale.bind(1, pid);
        removeSale.exec();

        // 如果需要重置共享者，则删除共享信息
        if (resetShares) {
            SQLite::Statement resetSharesStmt(*mSQLite, "DELETE FROM PlotShares WHERE mPlotID = ?");
            resetSharesStmt.bind(1, pid);
            resetSharesStmt.exec();
        }

        // Commit transaction
        mSQLite->exec("COMMIT");
        return true;
    } catch (SQLite::Exception& e) {
        mSQLite->exec("ROLLBACK");
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Transaction failed in buyPlotFromSale, SQLite::Exception: {}",
            e.what()
        );
        return false;
    }
}


// 联表查询获取玩家权限等级
PlotPermission PlotDBImpl::getPlayerPermission(UUID const& uid, PlotID const& pid, bool ignoreAdmin) {
    try {
        if (!ignoreAdmin && isAdmin(uid)) return PlotPermission::Admin;
        if (isPlotOwner(pid, uid)) return PlotPermission::Owner;
        if (isPlotSharedPlayer(pid, uid)) return PlotPermission::Shared;

        return PlotPermission::None; // 无权限
    }
    HandleSQLiteExceptionAndReturn(PlotPermission::None);
}


// ======================= Class PlotDB =======================

PlotPermission PlotDB::getPermission(UUID const& uuid, PlotID const& pid, bool ignoreAdmin, bool ignoreCache) {
    if (ignoreCache) return getImpl().getPlayerPermission(uuid, pid, ignoreAdmin);
    auto plot  = getCached(pid);
    auto share = getCached(pid, uuid);
    auto admin = getCached(uuid);
    if (admin.has_value()) {
        if (!ignoreAdmin && admin.value()) return PlotPermission::Admin;
    }
    if (plot.has_value()) {
        if (plot->mPlotOwner == uuid) return PlotPermission::Owner;
    }
    if (share.has_value()) {
        if (share->mSharedPlayer == uuid) return PlotPermission::Shared;
    }
    return PlotPermission::None;
}


PlotDBImpl& PlotDB::getImpl() { return *mImpl; }
PlotDB&     PlotDB::getInstance() {
    static PlotDB instance;
    return instance;
}


bool PlotDB::load() {
    if (mImpl != nullptr) return true;
    mImpl = std::make_unique<PlotDBImpl>();
    initCache(); // 初始化缓存
    return true;
}
bool PlotDB::resetCache(CacheType type) {
    switch (type) {
    case CacheType::Plot:
        mPlots.clear();
        initCache(CacheType::Plot);
        break;
    case CacheType::PlotShare:
        mPlotShares.clear();
        initCache(CacheType::PlotShare);
        break;
    case CacheType::Admin:
        mAdmins.clear();
        initCache(CacheType::Admin);
        break;
    default:
        mPlots.clear();
        mPlotShares.clear();
        mAdmins.clear();
        initCache();
        break;
    }
    return true;
}
bool PlotDB::initCache(CacheType type) {
    static bool initialized = false;
    if (initialized) return true;
    auto plots  = mImpl->getPlots();
    auto shares = mImpl->getSharedPlots();
    auto admins = mImpl->getAdmins();
    // init plots
    if (type == CacheType::Plot || type == CacheType::All)
        for (auto const& plot : plots) {
            cache(plot);
        }
    // init shares
    if (type == CacheType::PlotShare || type == CacheType::All)
        for (auto const& share : shares) {
            cache(share);
        }
    // init admins
    if (type == CacheType::Admin || type == CacheType::All)
        for (auto const& admin : admins) {
            cache(admin);
        }
    initialized = true;
    return true;
}


size_t PlotDB::hash(PlotID const& pid) { return std::hash<string>()(pid); }
size_t PlotDB::hash(PlotID const& pid, UUID const& uid) { return std::hash<string>()(pid + uid.asString()); }


bool PlotDB::cache(Plot const& plot) {
    mPlots[hash(plot.mPlotID)] = Plot{plot}; // copy
    return true;
}
bool PlotDB::cache(PlotShare const& share) {
    mPlotShares[hash(share.mPlotID, share.mSharedPlayer)] = PlotShare{share}; // copy
    return true;
}
bool PlotDB::cache(UUID const& uuid) {
    mAdmins[uuid] = true; // copy
    return true;
}
bool PlotDB::cache(size_t const& key, CacheType type, DynamicCache const& data) {
    switch (type) {
    case CacheType::Plot:
        mPlots[key] = std::get<Plot>(data); // copy
        break;
    case CacheType::PlotShare:
        mPlotShares[key] = std::get<PlotShare>(data); // copy
        break;
    case CacheType::Admin:
        mAdmins[key] = std::get<bool>(data); // copy
        break;
    default:
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Fail in {}, Invalid cache type: {}",
            __FUNCTION__,
            static_cast<int>(type)
        );
        return false;
    }
    return true;
}


bool PlotDB::hasCached(PlotID const& pid) { return mPlots.find(hash(pid)) != mPlots.end(); }
bool PlotDB::hasCached(PlotID const& pid, UUID const& uid) {
    return mPlotShares.find(hash(pid, uid)) != mPlotShares.end();
}
bool PlotDB::hasCached(UUID const& uuid) { return mAdmins.find(uuid) != mAdmins.end(); }
bool PlotDB::hasCached(size_t const& key, CacheType type) {
    switch (type) {
    case CacheType::Plot:
        return mPlots.find(key) != mPlots.end();
    case CacheType::PlotShare:
        return mPlotShares.find(key) != mPlotShares.end();
    case CacheType::Admin:
        return mAdmins.find(key) != mAdmins.end();
    default:
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Fail in {}, Invalid cache type: {}",
            __FUNCTION__,
            static_cast<int>(type)
        );
        return false;
    }
}


std::optional<Plot> PlotDB::getCached(PlotID const& id) {
    auto it = mPlots.find(hash(id));
    if (it == mPlots.end()) return std::nullopt;
    return it->second;
}
std::optional<PlotShare> PlotDB::getCached(PlotID const& id, UUID const& uid) {
    auto it = mPlotShares.find(hash(id, uid));
    if (it == mPlotShares.end()) return std::nullopt;
    return it->second;
}
std::optional<bool> PlotDB::getCached(UUID const& uuid) {
    auto it = mAdmins.find(uuid);
    if (it == mAdmins.end()) return std::nullopt;
    return it->second;
}
std::optional<DynamicCache> PlotDB::getCached(size_t const& key, CacheType type) {
    DynamicCache val;
    switch (type) {
    case CacheType::Plot: {
        auto it = mPlots.find(key);
        if (it == mPlots.end()) return std::nullopt;
        val = it->second;
    } break;
    case CacheType::PlotShare: {
        auto it = mPlotShares.find(key);
        if (it == mPlotShares.end()) return std::nullopt;
        val = it->second;
    } break;
    case CacheType::Admin: {
        auto it = mAdmins.find(key);
        if (it == mAdmins.end()) return std::nullopt;
        val = it->second;
    } break;
    default:
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Fail in {}, Invalid cache type: {}",
            __FUNCTION__,
            static_cast<int>(type)
        );
        return std::nullopt;
    }
    return val;
}


bool PlotDB::removeCached(PlotID const& pid) {
    auto it = mPlots.find(hash(pid));
    if (it == mPlots.end()) return false;
    mPlots.erase(it);
    return true;
}
bool PlotDB::removeCached(PlotID const& pid, UUID const& uid) {
    auto it = mPlotShares.find(hash(pid, uid));
    if (it == mPlotShares.end()) return false;
    mPlotShares.erase(it);
    return true;
}
bool PlotDB::removeCached(UUID const& uuid) {
    auto it = mAdmins.find(uuid);
    if (it == mAdmins.end()) return false;
    mAdmins.erase(it);
    return true;
}
bool PlotDB::removeCached(size_t const& key, CacheType type) {
    switch (type) {
    case CacheType::Plot:
        mPlots.erase(key);
        break;
    case CacheType::PlotShare:
        mPlotShares.erase(key);
        break;
    case CacheType::Admin:
        mAdmins.erase(key);
        break;
    default:
        my_plugin::MyPlugin::getInstance().getSelf().getLogger().error(
            "Fail in {}, Invalid cache type: {}",
            __FUNCTION__,
            static_cast<int>(type)
        );
        return false;
    }
    return true;
}


bool PlotDB::updateCachedPlotName(PlotID const& pid, string const& newName) {
    auto it = mPlots.find(hash(pid));
    if (it == mPlots.end()) return false;
    it->second.mPlotName = string(newName);
    return true;
}
bool PlotDB::updateCachedPlotOwner(PlotID const& pid, UUID const& newOwner) {
    auto it = mPlots.find(hash(pid));
    if (it == mPlots.end()) return false;
    it->second.mPlotOwner = UUID(newOwner);
    return true;
}


} // namespace plo::database