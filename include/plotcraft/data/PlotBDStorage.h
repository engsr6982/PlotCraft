#pragma once
#include "ll/api/data/KeyValueDB.h"
#include "plotcraft/Macro.h"
#include "plotcraft/data/PlotMetadata.h"
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

using string = std::string;

class PlotPos;

namespace plo::database {


class PlotBDStorage {
private:
    std::unique_ptr<ll::data::KeyValueDB> mDB;

    // Cache:
    std::vector<UUID>                                         mAdmins; // 管理
    std::unordered_map<PlotID, std::shared_ptr<PlotMetadata>> mPlots;  // 地皮

public:
    PlotBDStorage()                                = default;
    PlotBDStorage(const PlotBDStorage&)            = delete;
    PlotBDStorage& operator=(const PlotBDStorage&) = delete;

    PLAPI ll::data::KeyValueDB& getDB();
    PLAPI PlotBDStorage&        getInstance();

    PLAPI void load();
    PLAPI void save();

    PLAPI void initKey();

    PLAPI void tryConvertOldDB();

    // Admins
    PLAPI bool hasAdmin(const UUID& uuid) const;
    PLAPI bool isAdmin(const UUID& uuid) const;
    PLAPI bool addAdmin(const UUID& uuid);
    PLAPI bool delAdmin(const UUID& uuid);
    PLAPI std::vector<UUID> getAdmins() const;

    // Plots
    PLAPI bool hasPlot(const PlotID& id) const;

    PLAPI bool delPlot(const PlotID& id);

    PLAPI bool addPlot(PlotID const& id, UUID const& owner, int const& x, int const& z);
    PLAPI bool addPlot(std::shared_ptr<PlotMetadata> plot);

    PLAPI std::shared_ptr<PlotMetadata> getPlot(PlotID const& id) const;
    PLAPI std::vector<std::shared_ptr<PlotMetadata>> getPlots() const;
    PLAPI std::vector<std::shared_ptr<PlotMetadata>> getPlots(UUID const& owner) const;


    // 辅助API
    PLAPI std::vector<std::shared_ptr<PlotMetadata>> getSaleingPlots(); // 出售中的地皮

    PLAPI bool buyPlotFromSale(PlotID const& pid, UUID const& buyer, bool resetShares = true); // 购买出售中的地皮

    PLAPI PlotPermission
    getPlayerPermission(UUID const& uuid, PlotID const& pid, bool ignoreAdmin = false) const; // 获取玩家的权限
};


} // namespace plo::database