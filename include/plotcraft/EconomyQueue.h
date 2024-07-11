#pragma once
#include "data/PlotMetadata.h"
#include "mc/deps/core/mce/UUID.h"
#include "plotcraft/Macro.h"
#include "plotcraft/utils/Moneys.h"
#include "plugin/MyPlugin.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <utility>
#include <vector>


using UUID   = plo::data::UUID;
using json   = nlohmann::json;
namespace fs = std::filesystem;

namespace plo {


class EconomyQueue {
public:
    std::shared_ptr<std::vector<std::shared_ptr<std::pair<UUID, uint64_t>>>> mQueue; // 队列

    fs::path mPath; // 文件路径

    EconomyQueue()                               = default;
    EconomyQueue(const EconomyQueue&)            = delete;
    EconomyQueue& operator=(const EconomyQueue&) = delete;

    PLAPI static EconomyQueue& getInstance();

    PLAPI bool has(UUID const& target) const;

    PLAPI std::shared_ptr<std::pair<UUID, uint64_t>> get(UUID const& target) const;

    PLAPI bool set(UUID const target, int const val);

    PLAPI bool del(UUID const& target);

    PLAPI bool transfer(Player& target);

    PLAPI bool save();

    PLAPI bool load();
};

} // namespace plo