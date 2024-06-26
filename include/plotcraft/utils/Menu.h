#pragma once
#include "mc/world/actor/player/Player.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "plotcraft/Macro.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


using string = std::string;
using json   = nlohmann::json;
using path   = std::filesystem::path;


namespace plo::utils {

struct Menu {
    struct ButtonItem {
        string title        = ""; // 按钮标题
        string imageData    = ""; // 按钮图片 url 或 path
        string imageType    = ""; // 图片类型，url 或 path
        string callbackType = ""; // 按钮回调类型
        string callbackRun  = ""; // 按钮回调函数
    };

    string                  title   = PLUGIN_NAME;
    string                  content = "";
    std::vector<ButtonItem> buttons = {};

    PLAPI void                         sendTo(Player& player);
    PLAPI static std::unique_ptr<Menu> fromJSON(const json& json);
    PLAPI static std::unique_ptr<Menu> fromJsonFile(const string path);
    PLAPI static void                  fromJsonFile(Player& player);

    // static members
    PLAPI static path                                                     rootDir;
    PLAPI static std::unordered_map<string, std::function<void(Player&)>> functions;
    PLAPI static bool                                                     hasFunction(const string name);
};

} // namespace plo::utils