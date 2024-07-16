#include "plotcraft/data/PlotBDStorage.h"
#include "SQLiteCpp/Database.h"
#include "ll/api/data/KeyValueDB.h"
#include "nlohmann/json_fwd.hpp"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plugin/MyPlugin.h"
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>


#include "SQLiteCpp/SQLiteCpp.h"

#ifdef DEBUG
#define debugger(...) std::cout << "[Debug] " << __VA_ARGS__ << std::endl;
#else
#define debugger(...) ((void)0)
#endif

using namespace plo::utils;

namespace plo::data {

void PlotBDStorage::tryStartSaveThread() {
    static bool isStarted = false;
    if (isStarted) return;
    isStarted = true;
    std::thread([this]() {
        while (true) {
            debugger(" [" << Date{}.toString() << "] Saveing...");
            this->save();
            debugger(" [" << Date{}.toString() << "] Save done.");
            std::this_thread::sleep_for(std::chrono::minutes(2));
        }
    }).detach();
}

PlotBDStorage& PlotBDStorage::getInstance() {
    static PlotBDStorage instance;
    return instance;
}

ll::data::KeyValueDB& PlotBDStorage::getDB() { return *mDB; }


#define DB_PlayerSettingsKey "PlayerSettings"
#define DB_PlotAdminsKey     "PlotAdmins"

void PlotBDStorage::load() {
    if (!mDB) {
        mDB = std::make_unique<ll::data::KeyValueDB>(
            my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlotDBStorage"
        );
    }
    mAdmins.clear();
    mPlots.clear();
    initKey();
    tryConvertOldDB();

    // Load data from database
    auto* logger = &my_plugin::MyPlugin::getInstance().getSelf().getLogger();
    mDB->iter([this, logger](std::string_view key, std::string_view value) {
        try {
            if (key.starts_with("(") && key.ends_with(")")) {
                auto j   = nlohmann::json::parse(value);
                auto ptr = PlotMetadata::make();
                JsonHelper::jsonToStruct(j, *ptr);
                this->mPlots[ptr->getPlotID()] = ptr;

            } else if (key == DB_PlotAdminsKey) {
                auto j = nlohmann::json::parse(value);
                for (auto const& uuid : j) {
                    this->mAdmins.push_back(UUID(uuid.get<std::string>()));
                }

            } else if (key == DB_PlayerSettingsKey) {
                auto j = nlohmann::json::parse(value);
                for (auto const& [uuid, settings] : j.items()) {
                    PlayerSettingItem it;
                    JsonHelper::jsonToStruct(settings, it);
                    mPlayerSettings[UUID(uuid)] = it;
                }

            } else logger->warn("Unknown key in PlotBDStorage: {}", key);
        } catch (std::exception const& e) {
            logger->error("Fail in {}, error key: {}\n{}", __func__, key, e.what());
        } catch (...) {
            logger->fatal("Fail in {}, error key: {}", __func__, key);
        }
        return true;
    });

    logger->info("已加载 {} 条地皮数据", mPlots.size());
    logger->info("已加载 {} 位管理员数据", mAdmins.size());
    logger->info("已加载 {} 位玩家设置数据", mPlayerSettings.size());
}

void PlotBDStorage::save(PlotMetadata const& plot) {
    auto j = JsonHelper::structToJson(plot);
    mDB->set(plot.getPlotID(), j.dump());
}

void PlotBDStorage::save() {
    // Save PlotMetadata
    for (auto const& [id, ptr] : mPlots) {
        save(*ptr);
    }

    // Save PlotAdmins
    mDB->set(DB_PlotAdminsKey, JsonHelper::structToJson(mAdmins).dump());

    // Save PlayerSettings
    {
        nlohmann::json j = nlohmann::json::object();
        for (auto const& [uuid, settings] : mPlayerSettings) {
            j[uuid] = JsonHelper::structToJson(settings);
        }
        mDB->set(DB_PlayerSettingsKey, j.dump());
    }
}

void PlotBDStorage::initKey() {
    if (!mDB->has(DB_PlayerSettingsKey)) {
        mDB->set(DB_PlayerSettingsKey, "{}");
    }
    if (!mDB->has(DB_PlotAdminsKey)) {
        mDB->set(DB_PlotAdminsKey, "[]");
    }
}

namespace fs = std::filesystem;

void PlotBDStorage::tryConvertOldDB() {
    auto& mSelf  = my_plugin::MyPlugin::getInstance().getSelf();
    auto& logger = mSelf.getLogger();

    fs::path oldSQLitePath     = mSelf.getDataDir() / "PlotCraft.db";
    fs::path reNamedSQLitePath = mSelf.getDataDir() / "PlotCraft.db.bak";
    if (!fs::exists(oldSQLitePath)) return;
    fs::rename(oldSQLitePath, reNamedSQLitePath);

    logger.warn("[Convert] 检测到旧数据库文件，尝试转换到LevelDB...");

    try {
        SQLite::Database sptr(reNamedSQLitePath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

        // PlotAdmins
        SQLite::Statement query(sptr, "SELECT * FROM PlotAdmins");
        while (query.executeStep()) {
            auto uuid = query.getColumn(0).getText();
            mAdmins.push_back(UUID(uuid));
        }
        logger.info("[Convert] [PlotAdmins] 转换完成，共计 {} 条数据。", mAdmins.size());

        // Plots
        SQLite::Statement query2(sptr, "SELECT * FROM Plots");
        while (query2.executeStep()) {
            auto id    = query2.getColumn(0).getText();
            auto name  = query2.getColumn(1).getText();
            auto owner = query2.getColumn(2).getText();
            auto x     = query2.getColumn(3).getInt();
            auto z     = query2.getColumn(4).getInt();
            auto ptr   = PlotMetadata::make(id, owner, name, x, z);
            mPlots[id] = ptr;
        }
        logger.info("[Convert] [Plots] 转换完成，共计 {} 条数据。", mPlots.size());

        // PlotShares
        SQLite::Statement query3(sptr, "SELECT * FROM PlotShares");
        while (query3.executeStep()) {
            auto plotID       = query3.getColumn(0).getText();
            auto sharedPlayer = query3.getColumn(1).getText();
            auto sharedTime   = query3.getColumn(2).getText();

            auto ptr = getPlot(plotID);
            if (ptr) {
                ptr->mSharedPlayers.push_back({sharedPlayer, sharedTime});
            } else logger.warn("[Convert] [PlotShares] 未找到 {} 对应的地皮实例。", plotID);
        }
        logger.info("[Convert] [PlotShares] 转换完成，共计 {} 条数据。", mPlots.size());

        // PlotCommenets
        SQLite::Statement query4(sptr, "SELECT * FROM PlotCommenets");
        while (query4.executeStep()) {
            auto id      = query4.getColumn(0).getInt();
            auto plotID  = query4.getColumn(1).getText();
            auto player  = query4.getColumn(2).getText();
            auto time    = query4.getColumn(3).getText();
            auto content = query4.getColumn(4).getText();

            auto ptr = getPlot(plotID);
            if (ptr) {
                ptr->mComments.push_back({id, player, time, content});
            } else logger.warn("[Convert] [PlotCommenets] 未找到 {} 对应的地皮实例。", plotID);
        }
        logger.info("[Convert] [PlotCommenets] 转换完成，共计 {} 条数据。", mPlots.size());

        // PlotSales
        SQLite::Statement query5(sptr, "SELECT * FROM PlotSales");
        while (query5.executeStep()) {
            auto plotID = query5.getColumn(0).getText();
            auto price  = query5.getColumn(1).getInt();
            // auto time   = query5.getColumn(2); // 废弃

            auto ptr = getPlot(plotID);
            if (ptr) {
                ptr->setSaleStatus(true, price);
            } else logger.warn("[Convert] [PlotSales] 未找到 {} 对应的地皮实例。", plotID);
        }
        logger.info("[Convert] [PlotSales] 转换完成，共计 {} 条数据。", mPlots.size());

        logger.info("[Convert] 转换完成，旧数据库文件已备份至 {}", reNamedSQLitePath);
    } catch (SQLite::Exception const& e) {
        logger.error("[Convert] 转换旧数据库失败，SQLite::Exception: {}", e.what());
    } catch (std::exception const& e) {
        logger.error("[Convert] 转换旧数据库失败，std::exception: {}", e.what());
    } catch (...) {
        logger.error("[Convert] 转换旧数据库失败，未知错误");
    }
}


bool PlotBDStorage::hasAdmin(UUID const& uuid) const {
    return std::find(mAdmins.begin(), mAdmins.end(), uuid) != mAdmins.end();
}

bool PlotBDStorage::isAdmin(UUID const& uuid) const { return hasAdmin(uuid); }

bool PlotBDStorage::addAdmin(UUID const& uuid) {
    if (hasAdmin(uuid)) {
        return false;
    }
    mAdmins.push_back(UUID(uuid));
    return true;
}

bool PlotBDStorage::delAdmin(UUID const& uuid) {
    auto it = std::find(mAdmins.begin(), mAdmins.end(), uuid);
    if (it == mAdmins.end()) {
        return false;
    }
    mAdmins.erase(it);
    return true;
}

std::vector<UUID> PlotBDStorage::getAdmins() const { return mAdmins; }


bool PlotBDStorage::hasPlot(PlotID const& id) const { return mPlots.find(id) != mPlots.end(); }

bool PlotBDStorage::delPlot(PlotID const& id) {
    auto it = mPlots.find(id);
    if (it == mPlots.end()) {
        return false;
    }
    mPlots.erase(it);
    return true;
}

bool PlotBDStorage::addPlot(std::shared_ptr<PlotMetadata> ptr) {
    if (hasPlot(ptr->getPlotID())) {
        return false;
    }
    if (ptr->getPlotOwner().empty() || ptr->getPlotID().empty()) {
        throw std::runtime_error("Invalid plot metadata");
        return false;
    }

    mPlots[ptr->getPlotID()] = ptr;
    return true;
}

bool PlotBDStorage::addPlot(PlotID const& id, UUID const& owner, int x, int z) {
    auto ptr = PlotMetadata::make(id, owner, x, z);
    return addPlot(ptr);
}

std::shared_ptr<PlotMetadata> PlotBDStorage::getPlot(PlotID const& id) const {
    auto it = mPlots.find(id);
    if (it == mPlots.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<std::shared_ptr<PlotMetadata>> PlotBDStorage::getPlots() const {
    std::vector<std::shared_ptr<PlotMetadata>> res;
    for (auto const& [id, ptr] : mPlots) {
        res.push_back(ptr);
    }
    return res;
}

std::vector<std::shared_ptr<PlotMetadata>> PlotBDStorage::getPlots(UUID const& owner) const {
    std::vector<std::shared_ptr<PlotMetadata>> res;
    for (auto const& [id, ptr] : mPlots) {
        if (ptr->getPlotOwner() == owner) {
            res.push_back(ptr);
        }
    }
    return res;
}


bool PlotBDStorage::hasPlayerSetting(UUID const& uuid) const {
    return mPlayerSettings.find(uuid) != mPlayerSettings.end();
}
bool PlotBDStorage::initPlayerSetting(UUID const& uuid) {
    if (hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettings[uuid] = PlayerSettingItem{};
    return true;
}
bool PlotBDStorage::setPlayerSetting(UUID const& uuid, PlayerSettingItem const& setting) {
    if (!hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettings[uuid] = PlayerSettingItem{setting}; // copy
    return true;
}
PlayerSettingItem PlotBDStorage::getPlayerSetting(UUID const& uuid) const {
    auto it = mPlayerSettings.find(uuid);
    if (it == mPlayerSettings.end()) {
        return PlayerSettingItem{};
    }
    return it->second;
}


std::vector<std::shared_ptr<PlotMetadata>> PlotBDStorage::getSaleingPlots() const {
    std::vector<std::shared_ptr<PlotMetadata>> res;
    for (auto const& [id, ptr] : mPlots) {
        if (ptr->isSale()) {
            res.push_back(ptr);
        }
    }
    return res;
}

bool PlotBDStorage::buyPlotFromSale(PlotID const& id, UUID const& buyer, bool resetShares) {
    auto ptr = getPlot(id);
    if (!ptr) return false;

    ptr->setPlotOwner(buyer);
    ptr->setSaleStatus(false, 0);
    if (resetShares) ptr->resetSharedPlayers();

    return true;
}

PlotPermission PlotBDStorage::getPlayerPermission(UUID const& uuid, PlotID const& id, bool ignoreAdmin) const {
    if (!ignoreAdmin && isAdmin(uuid)) return PlotPermission::Admin;

    auto ptr = getPlot(id);
    if (!ptr) return PlotPermission::None;

    return ptr->getPlayerInThisPlotPermission(uuid);
}


} // namespace plo::data