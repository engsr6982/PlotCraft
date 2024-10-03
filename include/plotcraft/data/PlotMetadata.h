#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Global.h"
#include "plotcraft/math/PlotPos.h"
#include <memory>
#include <optional>
#include <vector>


namespace plo::data {


enum class PlotPermission : int { None = 0, Shared = 1, Owner = 2, Admin = 3 };

struct PlotCommentItem {
    CommentID mCommentID;
    UUIDs     mCommentPlayer;
    string    mCommentTime;
    string    mContent;
};
struct PlotShareItem {
    UUIDs  mSharedPlayer;
    string mSharedTime;
};

struct PlotPermissionTable {
    // 标记 [x] 为复用权限
    bool allowFireSpread{true};          // 火焰蔓延
    bool allowAttackDragonEgg{false};    // 攻击龙蛋
    bool allowFarmDecay{true};           // 耕地退化
    bool allowPistonPush{true};          // 活塞推动
    bool allowRedstoneUpdate{true};      // 红石更新
    bool allowExplode{false};            // 爆炸
    bool allowDestroy{false};            // 允许破坏
    bool allowWitherDestroy{false};      // 允许凋零破坏
    bool allowPlace{false};              // 允许放置 [x]
    bool allowAttackPlayer{false};       // 允许攻击玩家
    bool allowAttackAnimal{false};       // 允许攻击动物
    bool allowAttackMob{true};           // 允许攻击怪物
    bool allowOpenChest{false};          // 允许打开箱子
    bool allowPickupItem{false};         // 允许拾取物品
    bool allowThrowSnowball{true};       // 允许投掷雪球
    bool allowThrowEnderPearl{true};     // 允许投掷末影珍珠
    bool allowThrowEgg{true};            // 允许投掷鸡蛋
    bool allowThrowTrident{true};        // 允许投掷三叉戟
    bool allowDropItem{true};            // 允许丢弃物品
    bool allowShoot{false};              // 允许射击 [x]
    bool allowThrowPotion{false};        // 允许投掷药水 [x]
    bool allowRideEntity{false};         // 允许骑乘实体
    bool allowRideTrans{false};          // 允许骑乘矿车、船
    bool allowAxePeeled{false};          // 允许斧头去皮
    bool allowAttackEnderCrystal{false}; // 允许攻击末地水晶
    bool allowDestroyArmorStand{false};  // 允许破坏盔甲架

    bool useAnvil{false};            // 使用铁砧
    bool useBarrel{false};           // 使用木桶
    bool useBeacon{false};           // 使用信标
    bool useBed{false};              // 使用床
    bool useBell{false};             // 使用钟
    bool useBlastFurnace{false};     // 使用高炉
    bool useBrewingStand{false};     // 使用酿造台
    bool useCampfire{false};         // 使用营火
    bool useFiregen{false};          // 使用打火石
    bool useCartographyTable{false}; // 使用制图台
    bool useComposter{false};        // 使用堆肥桶
    bool useCraftingTable{false};    // 使用工作台
    bool useDaylightDetector{false}; // 使用阳光探测器
    bool useDispenser{false};        // 使用发射器
    bool useDropper{false};          // 使用投掷器
    bool useEnchantingTable{false};  // 使用附魔台
    bool useDoor{false};             // 使用门
    bool useFenceGate{false};        // 使用栅栏门
    bool useFurnace{false};          // 使用熔炉
    bool useGrindstone{false};       // 使用砂轮
    bool useHopper{false};           // 使用漏斗
    bool useJukebox{false};          // 使用唱片机
    bool useLoom{false};             // 使用织布机
    bool useStonecutter{false};      // 使用切石机
    bool useNoteBlock{false};        // 使用音符盒
    bool useShulkerBox{false};       // 使用潜影盒
    bool useSmithingTable{false};    // 使用锻造台
    bool useSmoker{false};           // 使用烟熏炉
    bool useTrapdoor{false};         // 使用活板门
    bool useLectern{false};          // 使用讲台
    bool useCauldron{false};         // 使用炼药锅
    bool useLever{false};            // 使用拉杆
    bool useButton{false};           // 使用按钮
    bool useRespawnAnchor{false};    // 使用重生锚
    bool useItemFrame{false};        // 使用物品展示框
    bool useFishingHook{false};      // 使用钓鱼竿
    bool useBucket{false};           // 使用桶
    bool usePressurePlate{false};    // 使用压力板
    bool useArmorStand{false};       // 使用盔甲架
    bool useBoneMeal{false};         // 使用骨粉
    bool useHoe{false};              // 使用锄头
    bool useShovel{false};           // 使用锹

    bool editFlowerPot{false}; // 编辑花盆
    bool editSign{false};      // 编辑告示牌
};

struct VertexPos {
    int x, y, z;

    BlockPos         toBlockPos() const { return BlockPos(x, y, z); }
    Vec3             toVec3() const { return Vec3(x, y, z); }
    static VertexPos fromBlockPos(BlockPos const& pos) { return VertexPos{pos.x, pos.y, pos.z}; }
    operator BlockPos() const { return toBlockPos(); }
    operator Vec3() const { return toVec3(); }
};


using PlotMetadataPtr = std::shared_ptr<class PlotMetadata>;
class PlotMetadata {
public:
    int version = METADATA_VERSION;

    // private:
    PlotID mPlotID;
    string mPlotName{""};
    UUIDs  mPlotOwner{""};
    int    mPlotX;
    int    mPlotZ;

    bool mIsSale{false}; // 是否出售
    int  mPrice{0};      // 出售价格

    PlotPermissionTable          mPermissionTable; // 权限表
    std::vector<PlotShareItem>   mSharedPlayers;   // 共享者列表
    std::vector<PlotCommentItem> mComments;        // 评论列表


    // v1.1.0
    bool mMerged{false};
    struct {
        int                    mMergeCount{0};  // 合并次数
        std::vector<VertexPos> mCurrentVertexs; // 当前顶点列表
        std::vector<PlotID> mMergedPlotIDs; // 合并的地皮ID列表 (这里的地皮都算为子地皮，get时映射到主地皮)
        std::vector<RoadID>  mMergedRoadIDs;  // 合并的道路ID列表
        std::vector<CrossID> mMergedCrossIDs; // 合并的交叉点ID列表
    } mMergedData;

public:
    // Constructors:
    PLAPI static PlotMetadataPtr make();
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUIDs const& owner, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUIDs const& owner, string const& name, int x, int z);


    // APIs:
    PLAPI bool isMerged() const;
    PLAPI void mergeData(PlotMetadataPtr const other, bool mergeComment = true, bool mergeSharedPlayer = false);
    PLAPI void updateMergeData(PlotPos const& newRange); // 更改后需调用 PlotDBStorage::refreshMergeMap 刷新映射表
    PLAPI bool setMergeCount(int count);

    PLAPI bool isOwner(UUIDs const& uuid) const;

    PLAPI bool setPlotName(string const& name);

    PLAPI bool setPlotID(PlotID const& plotID);

    PLAPI bool setX(int x);

    PLAPI bool setZ(int z);

    PLAPI bool setPlotOwner(UUIDs const& owner);

    PLAPI bool isSharedPlayer(UUIDs const& uuid) const;

    PLAPI bool addSharedPlayer(UUIDs const& uuid);

    PLAPI bool delSharedPlayer(UUIDs const& uuid);

    PLAPI bool resetSharedPlayers();

    PLAPI std::vector<PlotShareItem> getSharedPlayers() const;

    PLAPI bool hasComment(CommentID const& commentID) const;

    PLAPI bool isCommentOwner(CommentID const& commentID, UUIDs const& uuid) const;

    PLAPI bool addComment(UUIDs const& uuid, string const& content);

    PLAPI bool delComment(CommentID const& commentID);

    PLAPI bool resetComments();

    PLAPI bool setCommentContent(CommentID const& commentID, string const& content);

    PLAPI std::optional<PlotCommentItem> getComment(CommentID const& commentID) const;
    PLAPI std::vector<PlotCommentItem> getComments() const;
    PLAPI std::vector<PlotCommentItem> getComments(UUIDs const& uuid) const;

    PLAPI bool setSaleStatus(bool isSale);

    PLAPI bool setSaleStatus(bool isSale, int price);

    PLAPI bool setSalePrice(int price);

    PLAPI bool isSale() const;

    PLAPI int getSalePrice() const;

    PLAPI PlotID getPlotID() const;

    PLAPI string getPlotName() const;

    PLAPI UUIDs getPlotOwner() const;

    PLAPI int getX() const;

    PLAPI int getZ() const;

    PLAPI PlotPermission getPlayerInThisPlotPermission(UUIDs const& uuid) const; // 玩家在此地皮的权限

    PLAPI PlotPermissionTable&       getPermissionTable();
    PLAPI PlotPermissionTable const& getPermissionTableConst() const;

    PLAPI void save();

    PLAPI string toString() const;
};


} // namespace plo::data
