#include "DataBase.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

using UUID = plo::database::UUID;

namespace plo {


struct EconomyQueueItem {
    UUID from;   // 源玩家
    UUID to;     // 目标玩家
    int  amount; // 金额
};


class EconomyQueue {
public:
    std::shared_ptr<std::vector<std::unique_ptr<EconomyQueueItem>>> mQueue;

    EconomyQueue()                               = default;
    EconomyQueue(const EconomyQueue&)            = delete;
    EconomyQueue& operator=(const EconomyQueue&) = delete;

    static EconomyQueue& getInstance() {
        static EconomyQueue instance;
        return instance;
    }

    bool insert(UUID const& from, UUID const& to, int const& amount) {
        auto ptr = std::make_unique<EconomyQueueItem>(from, to, amount);
        mQueue->push_back(std::move(ptr));
        return true;
    }

    bool hasItem(UUID const& from, UUID const& to) {
        auto fn = std::find_if(mQueue->begin(), mQueue->end(), [from, to](auto const& item) {
            return item->from == from && item->to == to;
        });
        return fn != mQueue->end();
    }
};


} // namespace plo