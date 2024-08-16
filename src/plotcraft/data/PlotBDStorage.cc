#include "plotcraft/data/PlotBDStorage.h"
#include "ll/api/data/KeyValueDB.h"
#include "nlohmann/json_fwd.hpp"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>


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
            debugger("[" << Date{}.toString() << "] Saveing...");
            this->save();
            debugger("[" << Date{}.toString() << "] Save done.");
            std::this_thread::sleep_for(std::chrono::minutes(2));
        }
    }).detach();
}

ll::data::KeyValueDB& PlotBDStorage::getDB() { return *mDB; }
PlotBDStorage&        PlotBDStorage::getInstance() {
    static PlotBDStorage instance;
    return instance;
}


#define DB_PlayerSettingsKey "PlayerSettings"
#define DB_PlotAdminsKey     "PlotAdmins"
#define DB_ArchivedPrefix    "Archived_" // Archived_(0,1)

void PlotBDStorage::_initKey() {
    if (!mDB->has(DB_PlayerSettingsKey)) {
        mDB->set(DB_PlayerSettingsKey, "{}");
    }
    if (!mDB->has(DB_PlotAdminsKey)) {
        mDB->set(DB_PlotAdminsKey, "[]");
    }
}


void PlotBDStorage::load() {
    if (!mDB) {
        mDB = std::make_unique<ll::data::KeyValueDB>(
            my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlotDBStorage"
        );
    }
    mAdmins.clear();
    mPlots.clear();
    _initKey();

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
                JsonHelper::jsonToStructNoMerge(j, mAdmins);

            } else if (key == DB_PlayerSettingsKey) {
                auto j = nlohmann::json::parse(value);
                for (auto const& [uuid, settings] : j.items()) {
                    PlayerSettingItem it;
                    JsonHelper::jsonToStruct(settings, it); // load and merge fix values
                    mPlayerSettings[UUIDs(uuid)] = it;
                }
            }
        } catch (std::exception const& e) {
            logger->error("Fail in {}, error key: {}\n{}", __func__, key, e.what());
        } catch (...) {
            logger->fatal("Fail in {}, error key: {}", __func__, key);
        }
        return true;
    });

    logger->info("已加载 {}条 地皮数据", mPlots.size());
    logger->info("已加载 {}位 管理员数据", mAdmins.size());
    logger->info("已加载 {}位 玩家设置数据", mPlayerSettings.size());
}

void PlotBDStorage::save(PlotMetadata const& plot) {
    mDB->set(plot.getPlotID(), JsonHelper::structToJson(plot).dump());
}
void PlotBDStorage::save() {
    // PlotMetadata
    for (auto const& [id, ptr] : mPlots) save(*ptr);

    // PlotAdmins
    mDB->set(DB_PlotAdminsKey, JsonHelper::structToJson(mAdmins).dump());

    // PlayerSettings
    mDB->set(DB_PlayerSettingsKey, JsonHelper::structToJson(mPlayerSettings).dump());
}


bool PlotBDStorage::hasAdmin(UUIDs const& uuid) const {
    return std::find(mAdmins.begin(), mAdmins.end(), uuid) != mAdmins.end();
}
bool PlotBDStorage::isAdmin(UUIDs const& uuid) const { return hasAdmin(uuid); }

bool PlotBDStorage::addAdmin(UUIDs const& uuid) {
    if (hasAdmin(uuid)) {
        return false;
    }
    mAdmins.push_back(UUIDs(uuid));
    return true;
}

bool PlotBDStorage::delAdmin(UUIDs const& uuid) {
    auto it = std::find(mAdmins.begin(), mAdmins.end(), uuid);
    if (it == mAdmins.end()) {
        return false;
    }
    mAdmins.erase(it);
    return true;
}

std::vector<UUIDs> PlotBDStorage::getAdmins() const { return mAdmins; }


// Plots
bool PlotBDStorage::hasPlot(PlotID const& id) const { return mPlots.find(id) != mPlots.end(); }

bool PlotBDStorage::delPlot(PlotID const& id) {
    auto it = mPlots.find(id);
    if (it == mPlots.end()) {
        return false;
    }
    mPlots.erase(it);
    return true;
}

bool PlotBDStorage::addPlot(PlotMetadataPtr ptr) {
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

bool PlotBDStorage::addPlot(PlotID const& id, UUIDs const& owner, int x, int z) {
    auto ptr = PlotMetadata::make(id, owner, x, z);
    return addPlot(ptr);
}

PlotMetadataPtr PlotBDStorage::getPlot(PlotID const& id) const {
    auto it = mPlots.find(id);
    if (it == mPlots.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<PlotMetadataPtr> PlotBDStorage::getPlots() const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlots) {
        res.push_back(ptr);
    }
    return res;
}

std::vector<PlotMetadataPtr> PlotBDStorage::getPlots(UUIDs const& owner) const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlots) {
        if (ptr->getPlotOwner() == owner) {
            res.push_back(ptr);
        }
    }
    return res;
}


bool PlotBDStorage::hasPlayerSetting(UUIDs const& uuid) const {
    return mPlayerSettings.find(uuid) != mPlayerSettings.end();
}
bool PlotBDStorage::initPlayerSetting(UUIDs const& uuid) {
    if (hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettings[uuid] = PlayerSettingItem{};
    return true;
}
bool PlotBDStorage::setPlayerSetting(UUIDs const& uuid, PlayerSettingItem const& setting) {
    if (!hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettings[uuid] = PlayerSettingItem{setting}; // copy
    return true;
}
PlayerSettingItem PlotBDStorage::getPlayerSetting(UUIDs const& uuid) const {
    auto it = mPlayerSettings.find(uuid);
    if (it == mPlayerSettings.end()) {
        return PlayerSettingItem{};
    }
    return it->second;
}


std::vector<PlotMetadataPtr> PlotBDStorage::getSaleingPlots() const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlots) {
        if (ptr->isSale()) {
            res.push_back(ptr);
        }
    }
    return res;
}

bool PlotBDStorage::buyPlotFromSale(PlotID const& id, UUIDs const& buyer, bool resetShares) {
    auto ptr = getPlot(id);
    if (!ptr) return false;

    ptr->setPlotOwner(buyer);
    ptr->setSaleStatus(false, 0);
    if (resetShares) ptr->resetSharedPlayers();

    return true;
}

PlotPermission PlotBDStorage::getPlayerPermission(UUIDs const& uuid, PlotID const& id, bool ignoreAdmin) const {
    if (!ignoreAdmin && isAdmin(uuid)) return PlotPermission::Admin;

    auto ptr = getPlot(id);
    if (!ptr) return PlotPermission::None;

    return ptr->getPlayerInThisPlotPermission(uuid);
}


} // namespace plo::data