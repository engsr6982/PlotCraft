#pragma once
#include "ll/api/data/KeyValueDB.h"
#include "plotcraft/Macro.h"
#include "plotcraft/Version.h"
#include "plotcraft/data/PlotMetadata.h"
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>


using string = std::string;
class PlotPos;

namespace plo::data {

struct PlayerSettingItem {
    int version = SETTING_VERSION;

    bool showPlotTip; // 是否显示地皮提示
};

struct PlotMergedInfo {
    std::vector<PlotID> mMergedPlotIDs; // 被合并的地皮ID

    int mMergedPlotCount;           // 被合并的地皮数量
    int mMinX, mMinZ, mMaxX, mMaxZ; // 合并后的地皮范围
};


class PlotBDStorage {
private:
    std::unique_ptr<ll::data::KeyValueDB> mDB;

    // Cache:
    std::vector<UUID>                           mAdmins;         // 管理
    std::unordered_map<PlotID, PlotMetadataPtr> mPlots;          // 地皮
    std::unordered_map<UUID, PlayerSettingItem> mPlayerSettings; // 玩家设置

    // Plot Merge:
    std::unordered_map<PlotID, PlotMergedInfo> mMergedPlotInfo; // 合并地皮信息
    std::unordered_map<PlotID, PlotID>         mMergePlotIDMap; // 合并地皮映射

public:
    PlotBDStorage()                                = default;
    PlotBDStorage(const PlotBDStorage&)            = delete;
    PlotBDStorage& operator=(const PlotBDStorage&) = delete;

    PLAPI ll::data::KeyValueDB& getDB();
    PLAPI static PlotBDStorage& getInstance();

    PLAPI void load();
    PLAPI void save(); // 保存所有数据
    PLAPI void save(PlotMetadata const& plot);

    PLAPI void tryStartSaveThread(); // 启动自动保存线程

    PLAPI void _initKey();

    // Admins
    PLAPI bool hasAdmin(const UUID& uuid) const;
    PLAPI bool isAdmin(const UUID& uuid) const;
    PLAPI bool addAdmin(const UUID& uuid);
    PLAPI bool delAdmin(const UUID& uuid);
    PLAPI std::vector<UUID> getAdmins() const;


    // Plot Merge
    PLAPI bool   isMergedPlot(PlotID const& id) const;
    PLAPI PlotID getOwnerPlotID(PlotID const& id) const;

    PLAPI bool                  hasMergedPlotInfo(PlotID const& id) const;
    PLAPI PlotMergedInfo&       getMergedPlotInfo(PlotID const& id);
    PLAPI PlotMergedInfo const& getMergedPlotInfoConst(PlotID const& id) const;

    PLAPI bool tryMergePlot(PlotID const& sour, PlotID const& target);

    PLAPI bool archivePlot(PlotID const& id);


    // Plots
    PLAPI bool hasPlot(const PlotID& id) const;

    PLAPI bool delPlot(const PlotID& id);

    PLAPI bool addPlot(PlotID const& id, UUID const& owner, int x, int z);
    PLAPI bool addPlot(PlotMetadataPtr plot);

    PLAPI PlotMetadataPtr getPlot(PlotID const& id) const;
    PLAPI std::vector<PlotMetadataPtr> getPlots() const;
    PLAPI std::vector<PlotMetadataPtr> getPlots(UUID const& owner) const;

    // Player settings
    PLAPI bool              hasPlayerSetting(const UUID& uuid) const;
    PLAPI bool              initPlayerSetting(const UUID& uuid);
    PLAPI bool              setPlayerSetting(const UUID& uuid, const PlayerSettingItem& setting);
    PLAPI PlayerSettingItem getPlayerSetting(const UUID& uuid) const;

    // 辅助API
    PLAPI std::vector<PlotMetadataPtr> getSaleingPlots() const; // 出售中的地皮

    PLAPI bool buyPlotFromSale(PlotID const& pid, UUID const& buyer, bool resetShares = true); // 购买出售中的地皮

    PLAPI PlotPermission
    getPlayerPermission(UUID const& uuid, PlotID const& pid, bool ignoreAdmin = false) const; // 获取玩家的权限
};


} // namespace plo::data