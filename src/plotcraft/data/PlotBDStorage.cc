#include "plotcraft/data/PlotBDStorage.h"
#include "ll/api/data/KeyValueDB.h"
#include "nlohmann/json_fwd.hpp"
#include "plotcraft/utils/JsonHelper.h"
#include "plugin/MyPlugin.h"
#include <memory>
#include <stdexcept>
#include <vector>

using namespace plo::utils;

namespace plo::data {

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
                auto ptr = std::make_shared<PlotMetadata>();
                JsonHelper::jsonToStruct(j, *ptr);
                this->mPlots[ptr->getPlotID()] = ptr;
                logger->info("Loaded PlotMetadata: {}", key);

            } else if (key == DB_PlotAdminsKey) {
                auto j = nlohmann::json::parse(value);
                for (auto const& [uuid, name] : j.items()) {
                    this->mAdmins.push_back(UUID(uuid));
                }

            } else if (key == DB_PlayerSettingsKey) {
                // TODO

            } else logger->warn("Unknown key in PlotBDStorage: {}", key);
        } catch (std::exception const& e) {
            logger->error("Fail in {}, error key: {}\n{}", __func__, key, e.what());
        } catch (...) {
            logger->fatal("Fail in {}, error key: {}", __func__, key);
        }
        return true;
    });

    logger->info("Loaded {} Plots", mPlots.size());
    logger->info("Loaded {} PlotAdmins", mAdmins.size());
}

void PlotBDStorage::save(PlotMetadata const& plot) {
    auto j = JsonHelper::structToJson(plot);
    mDB->set(plot.getPlotID(), j.dump());
}

void PlotBDStorage::save() {
    for (auto const& [id, ptr] : mPlots) {
        save(*ptr);
    }

    // Save PlotAdmins
    mDB->set(DB_PlotAdminsKey, JsonHelper::structToJson(mAdmins).dump());
}

void PlotBDStorage::initKey() {
    if (!mDB->has(DB_PlayerSettingsKey)) {
        mDB->set(DB_PlayerSettingsKey, "{}");
    }
    if (!mDB->has(DB_PlotAdminsKey)) {
        mDB->set(DB_PlotAdminsKey, "[]");
    }
}

void PlotBDStorage::tryConvertOldDB() {
    // TODO
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
    auto ptr = std::make_shared<PlotMetadata>();
    ptr->setPlotID(id);
    ptr->setPlotOwner(owner);
    ptr->setX(x);
    ptr->setZ(z);
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
    auto ptr = getPlot(id);
    if (!ptr) return PlotPermission::None;

    if (!ignoreAdmin && isAdmin(uuid)) return PlotPermission::Admin;

    return ptr->getPlayerInThisPlotPermission(uuid);
}


} // namespace plo::data