#include "plotcraft/math/EnCodeID.h"

namespace plo {


EnCodeID EnCode::encode(int x, int z) {
    uint64_t ux       = static_cast<uint64_t>(std::abs(x));
    uint64_t uz       = static_cast<uint64_t>(std::abs(z));
    uint64_t signBits = 0;
    if (x >= 0) signBits |= (1ULL << 63);
    if (z >= 0) signBits |= (1ULL << 62);
    return signBits | (ux << 31) | (uz & 0x7FFFFFFF);
}
std::pair<int, int> EnCode::decode(EnCodeID id) {
    bool xPositive = (id & (1ULL << 63)) != 0;
    bool zPositive = (id & (1ULL << 62)) != 0;
    int  x         = static_cast<int>((id >> 31) & 0x7FFFFFFF);
    int  z         = static_cast<int>(id & 0x7FFFFFFF);
    if (!xPositive) x = -x;
    if (!zPositive) z = -z;
    return {x, z};
}

/**
原理：
    1. 将x和z的绝对值分别存储在ux和uz中
    2. 将x和z的正负号分别存储在signBits的第63位和第62位中
    3. 将ux左移31位，uz右移31位，然后进行或运算，得到最终的编码结果
    4. 解码时，将编码结果右移31位，得到ux和uz，然后根据signBits的第63位和第62位判断x和z的正负号，得到最终的x和z
    5. 由于ux和uz都是无符号整数，所以需要使用uint64_t类型来存储
 */


} // namespace plo