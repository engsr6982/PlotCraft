#pragma once
#include "mc/deps/core/mce/UUID.h"
#include "plotcraft/Macro.h"
#include "plotcraft/Version.h"
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>


using string = std::string;


namespace plo::data {


typedef string    PlotID; // PPos::getPlotID()
typedef mce::UUID UUIDm;
typedef string    UUIDs;
typedef int       CommentID;


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
    // Added
    bool allowFireSpread{true};       // 火焰蔓延
    bool allowAttackDragonEgg{false}; // 攻击龙蛋
    bool allowFarmDecay{true};        // 耕地退化
    bool allowPistonPush{true};       // 活塞推动

    // TODO：1.0.0
    bool allow_destroy{false};         // 允许破坏
    bool allow_entity_destroy{false};  // 允许实体破坏
    bool allow_place{false};           // 允许放置
    bool allow_attack_player{false};   // 允许攻击玩家
    bool allow_attack_animal{false};   // 允许攻击动物
    bool allow_attack_mobs{true};      // 允许攻击怪物
    bool allow_open_chest{false};      // 允许打开箱子
    bool allow_pickupitem{false};      // 允许拾取物品
    bool allow_dropitem{true};         // 允许丢弃物品
    bool allow_shoot{false};           // 允许射击
    bool use_anvil{false};             // 使用铁砧
    bool use_barrel{false};            // 使用木桶
    bool use_beacon{false};            // 使用信标
    bool use_bed{false};               // 使用床
    bool use_bell{false};              // 使用钟
    bool use_blast_furnace{false};     // 使用高炉
    bool use_brewing_stand{false};     // 使用酿造台
    bool use_campfire{false};          // 使用营火
    bool use_firegen{false};           // 使用火焰生成
    bool use_cartography_table{false}; // 使用制图台
    bool use_composter{false};         // 使用堆肥桶
    bool use_crafting_table{false};    // 使用工作台
    bool use_daylight_detector{false}; // 使用阳光探测器
    bool use_dispenser{false};         // 使用发射器
    bool use_dropper{false};           // 使用投掷器
    bool use_enchanting_table{false};  // 使用附魔台
    bool use_door{false};              // 使用门
    bool use_fence_gate{false};        // 使用栅栏门
    bool use_furnace{false};           // 使用熔炉
    bool use_grindstone{false};        // 使用砂轮
    bool use_hopper{false};            // 使用漏斗
    bool use_jukebox{false};           // 使用唱片机
    bool use_loom{false};              // 使用织布机
    bool use_stonecutter{false};       // 使用切石机
    bool use_noteblock{false};         // 使用音符盒
    bool use_shulker_box{false};       // 使用潜影盒
    bool use_smithing_table{false};    // 使用锻造台
    bool use_smoker{false};            // 使用烟熏炉
    bool use_trapdoor{false};          // 使用活板门
    bool use_lectern{false};           // 使用讲台
    bool use_cauldron{false};          // 使用炼药锅
    bool use_lever{false};             // 使用拉杆
    bool use_button{false};            // 使用按钮
    bool use_respawn_anchor{false};    // 使用重生锚
    bool use_item_frame{false};        // 使用物品展示框
    bool use_fishing_hook{false};      // 使用钓鱼竿
    bool use_bucket{false};            // 使用桶
    bool use_pressure_plate{false};    // 使用压力板
    bool use_armor_stand{false};       // 使用盔甲架
    bool eat{false};                   // 允许吃
    bool allow_throw_potion{false};    // 允许投掷药水
    bool allow_ride_entity{false};     // 允许骑乘实体
    bool allow_ride_trans{false};      // 允许骑乘传送门
    bool edit_flower_pot{false};       // 允许编辑花盆
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

public:
    // Constructors:
    PLAPI static PlotMetadataPtr make();
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUIDs const& owner, int x, int z);
    PLAPI static PlotMetadataPtr make(PlotID const& plotID, UUIDs const& owner, string const& name, int x, int z);


    // APIs:
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
