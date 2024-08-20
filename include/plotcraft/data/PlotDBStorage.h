#pragma once
#include "ll/api/data/KeyValueDB.h"
#include "plotcraft/Macro.h"
#include "plotcraft/Version.h"
#include "plotcraft/core/PlotPos.h"
#include "plotcraft/data/PlotMetadata.h"
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>


using string = std::string;

namespace plo::data {

struct PlayerSettingItem {
    int version = SETTING_VERSION;

    bool showPlotTip; // 是否显示地皮提示
};


class PlotDBStorage {
private:
    std::unique_ptr<ll::data::KeyValueDB> mDB;

    // Cache:
    std::vector<UUIDs>                           mAdmins;         // 管理
    std::unordered_map<PlotID, PlotMetadataPtr>  mPlots;          // 地皮
    std::unordered_map<UUIDs, PlayerSettingItem> mPlayerSettings; // 玩家设置

public:
    PlotDBStorage()                                = default;
    PlotDBStorage(const PlotDBStorage&)            = delete;
    PlotDBStorage& operator=(const PlotDBStorage&) = delete;

    PLAPI ll::data::KeyValueDB& getDB();
    PLAPI static PlotDBStorage& getInstance();

    PLAPI void load();
    PLAPI void save(); // 保存所有数据
    PLAPI void save(PlotMetadata const& plot);

    PLAPI void tryStartSaveThread(); // 启动自动保存线程

    PLAPI void _initKey();

    // Admins
    PLAPI bool hasAdmin(const UUIDs& uuid) const;
    PLAPI bool isAdmin(const UUIDs& uuid) const;
    PLAPI bool addAdmin(const UUIDs& uuid);
    PLAPI bool delAdmin(const UUIDs& uuid);
    PLAPI std::vector<UUIDs> getAdmins() const;

    // Plots
    PLAPI bool hasPlot(const PlotID& id) const;

    PLAPI bool delPlot(const PlotID& id);

    PLAPI bool addPlot(PlotID const& id, UUIDs const& owner, int x, int z);
    PLAPI bool addPlot(PlotMetadataPtr plot);

    PLAPI PlotMetadataPtr getPlot(PlotID const& id) const;
    PLAPI std::vector<PlotMetadataPtr> getPlots() const;
    PLAPI std::vector<PlotMetadataPtr> getPlots(UUIDs const& owner) const;

    // Player settings
    PLAPI bool              hasPlayerSetting(const UUIDs& uuid) const;
    PLAPI bool              initPlayerSetting(const UUIDs& uuid);
    PLAPI bool              setPlayerSetting(const UUIDs& uuid, const PlayerSettingItem& setting);
    PLAPI PlayerSettingItem getPlayerSetting(const UUIDs& uuid) const;

    // 辅助API
    PLAPI std::vector<PlotMetadataPtr> getSaleingPlots() const; // 出售中的地皮

    PLAPI bool buyPlotFromSale(PlotID const& pid, UUIDs const& buyer, bool resetShares = true); // 购买出售中的地皮

    PLAPI PlotPermission
    getPlayerPermission(UUIDs const& uuid, PlotID const& pid, bool ignoreAdmin = false) const; // 获取玩家的权限
};


} // namespace plo::data