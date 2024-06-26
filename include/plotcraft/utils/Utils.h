#pragma once
#include "plotcraft/Macro.h"
#include <fmt/format.h>
#include <iostream>
#include <ll/api/form/CustomForm.h>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>


using string = std::string;

namespace plo::utils {

template <typename T>
PLAPI inline string join(std::vector<T> const& vec, string const& splitter = ", ") {
    if (vec.empty()) return "";
    return std::accumulate(
        std::next(vec.begin()),
        vec.end(),
        std::to_string(vec[0]),
        [splitter](const string& a, const T& b) -> string { return a + splitter + std::to_string(b); }
    );
}

template <typename T>
PLAPI inline bool some(std::vector<T> const& vec, T const& it) {
    if (vec.empty()) return false;
    return std::find(vec.begin(), vec.end(), it) != vec.end();
}


PLAPI inline string toJson(const std::unordered_map<string, double>& map) {
    string json = "{";
    for (const auto& pair : map) {
        json += "\"" + pair.first + "\":" + std::to_string(pair.second) + ",";
    }
    json[json.size() - 1] = '}';
    return json;
}

PLAPI inline string toJson(const std::unordered_map<string, int>& map) {
    string json = "{";
    for (const auto& pair : map) {
        json += "\"" + pair.first + "\":" + std::to_string(pair.second) + ",";
    }
    json[json.size() - 1] = '}';
    return json;
}

PLAPI inline void DebugFormPrint(const ll::form::CustomFormResult& dt) {
#ifdef DEBUG
    std::cout << "\033[0m\033[1;35m"
              << "======================================================================================"
              << "\033[0m" << std::endl;
    for (auto [name, result] : *dt) {
        static auto logDebugResult = [&](const ll::form::CustomFormElementResult& var) {
            if (std::holds_alternative<uint64_t>(var)) {
                std::cout << "\033[0m\033[1;33m"
                          << "[CustomForm Debug] "
                          << "\033[0m\033[1;32m" << name << "\033[0m\033[1;35m    " << std::get<uint64_t>(var)
                          << "    \033[0m\033[1;36muint64_t"
                          << "\033[0m" << std::endl;
            } else if (std::holds_alternative<double>(var)) {
                std::cout << "\033[0m\033[1;33m"
                          << "[CustomForm Debug] "
                          << "\033[0m\033[1;32m" << name << "\033[0m\033[1;35m    " << std::get<double>(var)
                          << "    \033[0m\033[1;36mdouble"
                          << "\033[0m" << std::endl;
            } else if (std::holds_alternative<std::string>(var)) {
                std::cout << "\033[0m\033[1;33m"
                          << "[CustomForm Debug] "
                          << "\033[0m\033[1;32m" << name << "\033[0m\033[1;35m    " << std::get<std::string>(var)
                          << "    \033[0m\033[1;36mstring"
                          << "\033[0m" << std::endl;
            }
        };
        logDebugResult(result);
    }
#endif
}

} // namespace plo::utils