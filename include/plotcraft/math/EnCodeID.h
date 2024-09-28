#pragma once
#include <cstdint>
#include <utility>


namespace plo {


using EnCodeID = uint64_t;

class EnCode {
public:
    EnCode()                         = delete;
    EnCode(const EnCode&)            = delete;
    EnCode& operator=(const EnCode&) = delete;

    static EnCodeID encode(int x, int z);

    static std::pair<int, int> decode(EnCodeID id);
};


} // namespace plo