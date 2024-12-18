#pragma once
#include "ll/api/data/KeyValueDB.h"
#include "plotcraft/Global.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/math/PlotPos.h"
#include <memory>
#include <optional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace plot::data {

struct PlayerSettingItem {
    int version = SETTING_VERSION;

    bool showPlotTip; // 是否显示地皮提示
};


class PlotDBStorage {
private:
    std::unique_ptr<ll::data::KeyValueDB> mDB;

    // Cache:
    std::vector<UUIDs>                           mAdminList;         // 管理
    std::unordered_map<PlotID, PlotMetadataPtr>  mPlotList;          // 地皮
    std::unordered_map<UUIDs, PlayerSettingItem> mPlayerSettingList; // 玩家设置

    // MergeAPI:
    std::unordered_map<RoadID, PlotID>  mMergeRoadMap;  // 路ID => 主地皮ID
    std::unordered_map<CrossID, PlotID> mMergeCrossMap; // 十字路口ID => 主地皮ID
    std::unordered_map<PlotID, PlotID>  mMergedPlots;   // 被合并的地皮 => 主地皮ID

    // Functions:
    void _initKey();

public:
    PlotDBStorage()                                = default;
    PlotDBStorage(const PlotDBStorage&)            = delete;
    PlotDBStorage& operator=(const PlotDBStorage&) = delete;

    // Merge API:
    PLAPI void    _initClass(PlotRoad& road);
    PLAPI void    _initClass(PlotCross& cross);
    PLAPI bool    _initClass(PlotPos& pos);
    PLAPI Vertexs _getInitClassVertexs(PlotPos& pos); // 获取初始化类顶点 (PlotPos Vec3构造函数使用)

    PLAPI bool refreshMergeMap();
    PLAPI void _archivePlotData(PlotID const& id);

    // Instance API:
    PLAPI ll::data::KeyValueDB& getDB();
    PLAPI static PlotDBStorage& getInstance();

    // Read/Write API:
    PLAPI void load();
    PLAPI void save(); // 保存所有数据
    PLAPI void save(PlotMetadata const& plot);

    std::jthread mSaveThread;      // 保存线程
    PLAPI void   initSaveThread(); // 初始化保存线程

    // Admin API:
    PLAPI bool hasAdmin(const UUIDs& uuid) const;
    PLAPI bool isAdmin(const UUIDs& uuid) const;
    PLAPI bool addAdmin(const UUIDs& uuid);
    PLAPI bool delAdmin(const UUIDs& uuid);
    PLAPI std::vector<UUIDs> getAdmins() const;

    // Plot API:
    PLAPI bool hasPlot(const PlotID& id, bool ignoreMergePlot = false) const;
    PLAPI bool addPlot(PlotID const& id, UUIDs const& owner, int x, int z);
    PLAPI bool addPlot(PlotMetadataPtr plot);

    [[deprecated("This function is not recommended unless it is necessary")]] PLAPI bool delPlot(const PlotID& id);

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

    PLAPI std::optional<PlotPos> findUnownedPlot() const;

    // 特殊数据 Key (统一定义Key，请勿修改)
    string const DB_ArchivedPrefix    = "Archived_";
    string const DB_PlotAdminsKey     = "PlotAdmins";
    string const DB_PlayerSettingsKey = "PlayerSettings";
};


} // namespace plot::data