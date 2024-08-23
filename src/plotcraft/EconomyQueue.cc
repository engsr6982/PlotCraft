#include "plotcraft/EconomyQueue.h"


namespace plo {


EconomyQueue& EconomyQueue::getInstance() {
    static EconomyQueue instance;
    return instance;
}

bool EconomyQueue::has(UUIDs const& target) const {
    return std::find_if(mQueue->begin(), mQueue->end(), [&target](auto const& pair) { return pair->first == target; })
        != mQueue->end();
}

std::shared_ptr<std::pair<UUIDs, uint64_t>> EconomyQueue::get(UUIDs const& target) const {
    auto it =
        std::find_if(mQueue->begin(), mQueue->end(), [&target](auto const& pair) { return pair->first == target; });
    if (it == mQueue->end()) {
        return nullptr;
    }
    return *it;
}

bool EconomyQueue::set(UUIDs const target, int const val) {
    auto it =
        std::find_if(mQueue->begin(), mQueue->end(), [&target](auto const& pair) { return pair->first == target; });
    if (it != mQueue->end()) {
        (*it)->second += val; // 累加
        save();
        return true;
    } else {
        auto ptr = std::make_shared<std::pair<UUIDs, uint64_t>>(std::make_pair(target, val));
        mQueue->push_back(ptr);
        save();
        return true;
    }
}

bool EconomyQueue::del(UUIDs const& target) {
    auto it =
        std::find_if(mQueue->begin(), mQueue->end(), [&target](auto const& pair) { return pair->first == target; });
    if (it != mQueue->end()) {
        mQueue->erase(it);
        save();
        return true;
    }
    return false;
}

bool EconomyQueue::transfer(Player& target) {
    UUIDs uid = target.getUuid().asString();
    if (!has(uid)) return false;

    auto&      ms  = utils::EconomySystem::getInstance();
    auto       ptr = get(uid);
    bool const ok  = ms.add(target, ptr->second);
    if (ok) {
        del(uid);
        return true;
    }
    return false;
}

bool EconomyQueue::save() {
    /*
        [
            [
                "UUIDs",
                000000000000
            ]
        ]
     */
    json j = json::array();
    for (auto const& pair : *mQueue) {
        j.push_back({pair->first, pair->second});
    }
    std::ofstream ofs(mPath);
    ofs << j.dump(4);
    ofs.close();
    return true;
};

bool EconomyQueue::load() {
    if (mQueue != nullptr) {
        throw std::runtime_error("EconomyQueue has already been initialized.");
    }
    mPath  = my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "EconomyQueue.json";
    mQueue = std::make_shared<std::vector<std::shared_ptr<std::pair<UUIDs, uint64_t>>>>();

    if (!fs::exists(mPath)) {
        // create father directory if not exist
        fs::create_directory(mPath.parent_path());
        save(); // save empty queue to file
    }

    std::ifstream ifs(mPath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open EconomyQueue file.");
    }
    json j;
    ifs >> j;
    ifs.close();
    for (auto const& item : j) {
        mQueue->push_back(std::make_shared<std::pair<UUIDs, uint64_t>>(std::make_pair(UUIDs(item[0]), item[1])));
    }
    return true;
}


} // namespace plo