#pragma once
#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include "nlohmann/json.hpp"
#include <iostream>


using string = std::string;
using json   = nlohmann::json;


namespace plo::utils {


class JsonHelper {
public:
    JsonHelper()                             = delete;
    JsonHelper(const JsonHelper&)            = delete;
    JsonHelper& operator=(const JsonHelper&) = delete;
    ~JsonHelper()                            = delete;

    template <class T>
    static json structToJson(T& obj) {
        return ll::reflection::serialize<nlohmann::ordered_json>(obj).value();
    }

    template <class T>
    static void jsonToStruct(json const& j, T& obj) {
        ll::reflection::deserialize(obj, j).value();
    }

    template <class T>
    static string structToJsonString(T& obj, int indent = -1) {
        return structToJson(obj).dump(indent);
    }
};


} // namespace plo::utils