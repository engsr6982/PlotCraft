#pragma once
#include <string>
#include <unordered_map>

namespace plot::event {

enum class MapType : int {
    UseItemOn     = 0, // 使用物品
    InteractBlock = 1, // 方块交互
    AnimalEntity  = 2, // 动物实体
    MobEntity     = 3  // 生物实体
};

// 运行时表
// 用于大量if时预检查字符串是否存在
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


} // namespace plot::event