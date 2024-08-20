#pragma once
#include <string>
#include <unordered_map>

namespace plo::event {

enum class MapType : int {
    UseItem         = 0, // 使用物品
    BlockInteracted = 1, // 方块交互
    ItemWhiteList   = 2, // 物品白名单
    SpecialAttack   = 3, // 特殊攻击
    AnimalEntity    = 4, // 动物实体
    MobEntity       = 5  // 生物实体
};

struct RuntimeMap {
public:
    RuntimeMap()                             = delete;
    RuntimeMap(RuntimeMap&&)                 = delete;
    RuntimeMap(const RuntimeMap&)            = delete;
    RuntimeMap& operator=(RuntimeMap&&)      = delete;
    RuntimeMap& operator=(const RuntimeMap&) = delete;

    static std::unordered_map<MapType, std::vector<std::string>> mData;

    static bool has(MapType type, std::string const& typeName);
};


} // namespace plo::event