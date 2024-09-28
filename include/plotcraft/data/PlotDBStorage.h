#pragma once
#include "ll/api/data/KeyValueDB.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/math/PlotPos.h"
#include <memory>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace plo::data {

struct PlayerSettingItem {
    int version = SETTING_VERSION;

    bool showPlotTip; // 是否显示地皮提示
};


class PlotDBStorage {
private:
    std::unique_ptr<ll::data::KeyValueDB> mDB;

    bool mThreadRunning{false};      // 保存线程是否正在运行
    bool mThreadRequiredExit{false}; // 保存线程是否需要退出

    // Cache:
    std::vector<UUIDs>                           mAdminList;         // 管理
    std::unordered_map<PlotID, PlotMetadataPtr>  mPlotList;          // 地皮
    std::unordered_map<UUIDs, PlayerSettingItem> mPlayerSettingList; // 玩家设置

    // MergeAPI:
    std::unordered_map<RoadID, PlotID>  mMergeRoadMap;  // 路ID => 地皮ID
    std::unordered_map<CrossID, PlotID> mMergeCrossMap; // 十字路口ID => 地皮ID
    std::unordered_set<PlotID>          mMergedPlots;   // 已合并的地皮

    // Functions:
    void _initKey();

public:
    PlotDBStorage()                                = default;
    PlotDBStorage(const PlotDBStorage&)            = delete;
    PlotDBStorage& operator=(const PlotDBStorage&) = delete;

    // Merge API:
    PLAPI void initClass(PlotRoad& road);
    // PLAPI void initClass(PlotCross& cross); // todo
    // PLAPI void initClass(PlotPos& pos);     // todo

    // Instance API:
    PLAPI ll::data::KeyValueDB& getDB();
    PLAPI static PlotDBStorage& getInstance();

    // Read/Write API:
    PLAPI void load();
    PLAPI void save(); // 保存所有数据
    PLAPI void save(PlotMetadata const& plot);

    PLAPI void initSaveThread(); // 初始化保存线程
    PLAPI bool isSaveThreadRunning() const;
    PLAPI void stopSaveThread(); // 停止保存线程

    // Admin API:
    PLAPI bool hasAdmin(const UUIDs& uuid) const;
    PLAPI bool isAdmin(const UUIDs& uuid) const;
    PLAPI bool addAdmin(const UUIDs& uuid);
    PLAPI bool delAdmin(const UUIDs& uuid);
    PLAPI std::vector<UUIDs> getAdmins() const;

    // Plot API:
    PLAPI bool hasPlot(const PlotID& id) const;
    PLAPI bool delPlot(const PlotID& id);
    PLAPI bool addPlot(PlotID const& id, UUIDs const& owner, int x, int z);
    PLAPI bool addPlot(PlotMetadataPtr plot);

    PLAPI PlotMetadataPtr getPlot(PlotID const& id) const;
    PLAPI std::vector<PlotMetadataPtr> getPlots() const;
    PLAPI std::vector<PlotMetadataPtr> getPlots(UUIDs const& owner) const;

    // PlayerSetting API:
    PLAPI bool              hasPlayerSetting(const UUIDs& uuid) const;
    PLAPI bool              initPlayerSetting(const UUIDs& uuid);
    PLAPI bool              setPlayerSetting(const UUIDs& uuid, const PlayerSettingItem& setting);
    PLAPI PlayerSettingItem getPlayerSetting(const UUIDs& uuid) const;

    // Other API:
    PLAPI std::vector<PlotMetadataPtr> getSaleingPlots() const; // 出售中的地皮

    PLAPI bool buyPlotFromSale(PlotID const& pid, UUIDs const& buyer, bool resetShares = true); // 购买出售中的地皮

    PLAPI PlotPermission getPlayerPermission(UUIDs const& uuid, PlotID const& pid, bool ignoreAdmin = false) const;
};


} // namespace plo::data