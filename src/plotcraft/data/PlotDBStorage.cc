#include "plotcraft/data/PlotDBStorage.h"
#include "ll/api/data/KeyValueDB.h"
#include "nlohmann/json_fwd.hpp"
#include "plotcraft/math/PlotPos.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>


using namespace plo::utils;

namespace plo::data {

bool PlotDBStorage::isSaveThreadRunning() const { return mThreadRunning; }
void PlotDBStorage::stopSaveThread() { mThreadRequiredExit = true; }
void PlotDBStorage::initSaveThread() {
    if (mThreadRunning) return;
    mThreadRunning = true;
    std::thread([this]() {
        while (!this->mThreadRequiredExit) {
            this->save();
            std::this_thread::sleep_for(std::chrono::minutes(2));
        }
        this->mThreadRunning      = false; // 重置标志位
        this->mThreadRequiredExit = false; // 重置标志位
    }).detach();
}


ll::data::KeyValueDB& PlotDBStorage::getDB() { return *mDB; }
PlotDBStorage&        PlotDBStorage::getInstance() {
    static PlotDBStorage instance;
    return instance;
}


#define DB_PlayerSettingsKey "PlayerSettings"
#define DB_PlotAdminsKey     "PlotAdmins"
#define DB_ArchivedPrefix    "Archived_" // Archived_(0,1)

void PlotDBStorage::_initKey() {
    if (!mDB->has(DB_PlayerSettingsKey)) {
        mDB->set(DB_PlayerSettingsKey, "{}");
    }
    if (!mDB->has(DB_PlotAdminsKey)) {
        mDB->set(DB_PlotAdminsKey, "[]");
    }
}


void PlotDBStorage::load() {
    if (!mDB) {
        mDB = std::make_unique<ll::data::KeyValueDB>(
            my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlotDBStorage"
        );
    }
    mAdminList.clear();
    mPlotList.clear();
    _initKey();

    // Load data from database
    auto* logger = &my_plugin::MyPlugin::getInstance().getSelf().getLogger();
    mDB->iter([this, logger](std::string_view key, std::string_view value) {
        try {
            if (key.starts_with("(") && key.ends_with(")")) {
                auto j   = nlohmann::json::parse(value);
                auto ptr = PlotMetadata::make();
                JsonHelper::jsonToStruct(j, *ptr);
                this->mPlotList[ptr->getPlotID()] = ptr;

            } else if (key == DB_PlotAdminsKey) {
                auto j = nlohmann::json::parse(value);
                JsonHelper::jsonToStructNoMerge(j, mAdminList);

            } else if (key == DB_PlayerSettingsKey) {
                auto j = nlohmann::json::parse(value);
                for (auto const& [uuid, settings] : j.items()) {
                    PlayerSettingItem it;
                    JsonHelper::jsonToStruct(settings, it); // load and merge fix values
                    mPlayerSettingList[UUIDs(uuid)] = it;
                }
            }
        } catch (std::exception const& e) {
            logger->error("Fail in {}, error key: {}\n{}", __func__, key, e.what());
        } catch (...) {
            logger->fatal("Fail in {}, error key: {}", __func__, key);
        }
        return true;
    });

    logger->info("已加载 {}条 地皮数据", mPlotList.size());
    logger->info("已加载 {}位 管理员数据", mAdminList.size());
    logger->info("已加载 {}位 玩家设置数据", mPlayerSettingList.size());

    // Init Map
    logger->info("初始化运行时必要的表...");
    for (auto const& [id, ptr] : mPlotList) {
        if (!ptr->isMerged()) {
            continue;
        }
        mMergedPlots.emplace(id); // self

        auto& data = ptr->mMergedData;
        for (auto const& p : data.mMergedPlotIDs) {
            mMergedPlots.emplace(p);
        }
        for (auto const& r : data.mMergedRoadIDs) {
            mMergeRoadMap.emplace(id, r);
        }
        for (auto const& c : data.mMergedCrossIDs) {
            mMergeCrossMap.emplace(id, c);
        }
    }
    logger->info("初始化完成");
    logger->debug("mMergedPlots: {}", mMergedPlots.size());
    logger->debug("mMergeRoadMap: {}", mMergeRoadMap.size());
    logger->debug("mMergeCrossMap: {}", mMergeCrossMap.size());
}

void PlotDBStorage::save(PlotMetadata const& plot) {
    mDB->set(plot.getPlotID(), JsonHelper::structToJson(plot).dump());
}
void PlotDBStorage::save() {
    // PlotMetadata
    for (auto const& [id, ptr] : mPlotList) save(*ptr);

    // PlotAdmins
    mDB->set(DB_PlotAdminsKey, JsonHelper::structToJson(mAdminList).dump());

    // PlayerSettings
    mDB->set(DB_PlayerSettingsKey, JsonHelper::structToJson(mPlayerSettingList).dump());
}


bool PlotDBStorage::hasAdmin(UUIDs const& uuid) const {
    return std::find(mAdminList.begin(), mAdminList.end(), uuid) != mAdminList.end();
}
bool PlotDBStorage::isAdmin(UUIDs const& uuid) const { return hasAdmin(uuid); }

bool PlotDBStorage::addAdmin(UUIDs const& uuid) {
    if (hasAdmin(uuid)) {
        return false;
    }
    mAdminList.push_back(UUIDs(uuid));
    return true;
}

bool PlotDBStorage::delAdmin(UUIDs const& uuid) {
    auto it = std::find(mAdminList.begin(), mAdminList.end(), uuid);
    if (it == mAdminList.end()) {
        return false;
    }
    mAdminList.erase(it);
    return true;
}

std::vector<UUIDs> PlotDBStorage::getAdmins() const { return mAdminList; }


// Plots
bool PlotDBStorage::hasPlot(PlotID const& id) const { return mPlotList.find(id) != mPlotList.end(); }

bool PlotDBStorage::delPlot(PlotID const& id) {
    auto it = mPlotList.find(id);
    if (it == mPlotList.end()) {
        return false;
    }
    mPlotList.erase(it);
    return true;
}

bool PlotDBStorage::addPlot(PlotMetadataPtr ptr) {
    if (hasPlot(ptr->getPlotID())) {
        return false;
    }
    if (ptr->getPlotOwner().empty() || ptr->getPlotID().empty()) {
        throw std::runtime_error("Invalid plot metadata");
        return false;
    }

    mPlotList[ptr->getPlotID()] = ptr;
    return true;
}

bool PlotDBStorage::addPlot(PlotID const& id, UUIDs const& owner, int x, int z) {
    auto ptr = PlotMetadata::make(id, owner, x, z);
    return addPlot(ptr);
}

PlotMetadataPtr PlotDBStorage::getPlot(PlotID const& id) const {
    auto it = mPlotList.find(id);
    if (it == mPlotList.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<PlotMetadataPtr> PlotDBStorage::getPlots() const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlotList) {
        res.push_back(ptr);
    }
    return res;
}

std::vector<PlotMetadataPtr> PlotDBStorage::getPlots(UUIDs const& owner) const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlotList) {
        if (ptr->getPlotOwner() == owner) {
            res.push_back(ptr);
        }
    }
    return res;
}


bool PlotDBStorage::hasPlayerSetting(UUIDs const& uuid) const {
    return mPlayerSettingList.find(uuid) != mPlayerSettingList.end();
}
bool PlotDBStorage::initPlayerSetting(UUIDs const& uuid) {
    if (hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettingList[uuid] = PlayerSettingItem{};
    return true;
}
bool PlotDBStorage::setPlayerSetting(UUIDs const& uuid, PlayerSettingItem const& setting) {
    if (!hasPlayerSetting(uuid)) {
        return false;
    }
    mPlayerSettingList[uuid] = PlayerSettingItem{setting}; // copy
    return true;
}
PlayerSettingItem PlotDBStorage::getPlayerSetting(UUIDs const& uuid) const {
    auto it = mPlayerSettingList.find(uuid);
    if (it == mPlayerSettingList.end()) {
        return PlayerSettingItem{};
    }
    return it->second;
}


std::vector<PlotMetadataPtr> PlotDBStorage::getSaleingPlots() const {
    std::vector<PlotMetadataPtr> res;
    for (auto const& [id, ptr] : mPlotList) {
        if (ptr->isSale()) {
            res.push_back(ptr);
        }
    }
    return res;
}

bool PlotDBStorage::buyPlotFromSale(PlotID const& id, UUIDs const& buyer, bool resetShares) {
    auto ptr = getPlot(id);
    if (!ptr) return false;

    ptr->setPlotOwner(buyer);
    ptr->setSaleStatus(false, 0);
    if (resetShares) ptr->resetSharedPlayers();

    return true;
}

PlotPermission PlotDBStorage::getPlayerPermission(UUIDs const& uuid, PlotID const& id, bool ignoreAdmin) const {
    if (!ignoreAdmin && isAdmin(uuid)) return PlotPermission::Admin;

    auto ptr = getPlot(id);
    if (!ptr) return PlotPermission::None;

    return ptr->getPlayerInThisPlotPermission(uuid);
}


void PlotDBStorage::_initClass(PlotRoad& road) { road.mIsMergedPlot = this->mMergeRoadMap.contains(road.getRoadID()); }
void PlotDBStorage::_initClass(PlotCross& cross) {
    cross.mIsMergedPlot = this->mMergeCrossMap.contains(cross.getCrossID());
}


} // namespace plo::data