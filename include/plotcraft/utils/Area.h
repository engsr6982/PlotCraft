#pragma once
#include "Mc.h"
#include "Utils.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Global.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>


// disable C4244
#pragma warning(disable : 4244)
using string = std::string;

namespace plot::area {

// 2D/3D 圆/正方体/中心正方体
struct Circle2 {
    double centerX, centerZ;
    double pointX, pointZ;
    double width;
};
struct Circle3 {
    double centerX, centerY, centerZ;
    double pointX, pointY, pointZ;
    double width;
};
struct Square2 {
    double leftTopX, leftTopZ;
    double rightBottomX, rightBottomZ;
    double pointX, pointZ;
};
struct Square3 {
    double leftTopX, leftTopY, leftTopZ;
    double rightBottomX, rightBottomY, rightBottomZ;
    double pointX, pointY, pointZ;
};
struct CenteredSquare2 {
    double centerX, centerZ;
    double pointX, pointZ;
    double width;
};
struct CenteredSquare3 {
    double centerX, centerY, centerZ;
    double pointX, pointY, pointZ;
    double width;
};


// 2D/3D Circle
PLAPI inline bool isInside(const Circle2& reg) {
    double distance = std::sqrt(std::pow(reg.centerX - reg.pointX, 2) + std::pow(reg.centerZ - reg.pointZ, 2));
    return distance <= reg.width;
}
PLAPI inline bool isInside(const Circle3& reg) {
    double distance = std::sqrt(
        std::pow(reg.centerX - reg.pointX, 2) + std::pow(reg.centerY - reg.pointY, 2)
        + std::pow(reg.centerZ - reg.pointZ, 2)
    );
    return distance <= reg.width;
}
// 2D/3D Square
PLAPI inline bool isInside(const Square2& reg) {
    double minX = std::min(reg.leftTopX, reg.rightBottomX);
    double maxX = std::max(reg.leftTopX, reg.rightBottomX);
    double minZ = std::max(reg.leftTopZ, reg.rightBottomZ);
    double maxZ = std::min(reg.leftTopZ, reg.rightBottomZ);
    if (reg.pointX < minX || reg.pointX > maxX) return false;
    if (reg.pointZ > minZ || reg.pointZ < maxZ) return false;
    return true;
}
PLAPI inline bool isInside(const Square3& reg) {
    double minX = std::min(reg.leftTopX, reg.rightBottomX);
    double maxX = std::max(reg.leftTopX, reg.rightBottomX);
    double minY = std::max(reg.leftTopY, reg.rightBottomY);
    double maxY = std::min(reg.leftTopY, reg.rightBottomY);
    double minZ = std::min(reg.leftTopZ, reg.rightBottomZ);
    double maxZ = std::max(reg.leftTopZ, reg.rightBottomZ);
    if (reg.pointX < minX || reg.pointX > maxX) return false;
    if (reg.pointY > minY || reg.pointY < maxY) return false;
    if (reg.pointZ < minZ || reg.pointZ > maxZ) return false;
    return true;
}
// 2D/3D CenteredSquare
PLAPI inline bool isInside(const CenteredSquare2& reg) {
    double minX = reg.centerX - reg.width;
    double maxX = reg.centerX + reg.width;
    double minZ = reg.centerZ - reg.width;
    double maxZ = reg.centerZ + reg.width;
    return reg.pointX >= minX && reg.centerX <= maxX && reg.pointZ >= minZ && reg.centerZ <= maxZ;
}
PLAPI inline bool isInside(const CenteredSquare3& reg) {
    double minX = reg.centerX - reg.width;
    double maxX = reg.centerX + reg.width;
    double minY = reg.centerY - reg.width;
    double maxY = reg.centerY + reg.width;
    double minZ = reg.centerZ - reg.width;
    double maxZ = reg.centerZ + reg.width;
    return reg.pointX >= minX && reg.pointX <= maxX && reg.pointY >= minY && reg.pointY <= maxY && reg.pointZ >= minZ
        && reg.pointZ <= maxZ;
}


struct Boundary {
    string axis;     // 边界轴 x, y, z
    double boundary; // 边界位置
    double value;    // 当前超出边界的值

    Boundary() {}
    Boundary(string axis, double boundary, double value) : axis(axis), boundary(boundary), value(value) {}
};

// 2D/3D Circle
PLAPI inline Boundary getBoundary(const Circle2& reg) {
    Boundary result;
    double   dx       = reg.pointX - reg.centerX;
    double   dz       = reg.pointZ - reg.centerZ;
    double   distance = std::sqrt(dx * dx + dz * dz);
    if (distance > reg.width) {
        double angle = std::atan2(dz, dx);
        result.axis  = std::abs(dx) > std::abs(dz) ? 'x' : 'z';
        result.value = result.axis == "x" ? reg.pointX : reg.pointZ;
        result.boundary =
            result.axis == "x" ? reg.centerX + reg.width * std::cos(angle) : reg.centerZ + reg.width * std::sin(angle);
    }
    return result;
}
PLAPI inline Boundary getBoundary(const Circle3& reg) {
    Boundary result;
    double   dx       = reg.pointX - reg.centerX;
    double   dy       = reg.pointY - reg.centerY;
    double   dz       = reg.pointZ - reg.centerZ;
    double   distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (distance > reg.width) {
        double ratio    = reg.width / distance;
        string axis     = std::abs(dx) > std::abs(dy) ? (std::abs(dx) > std::abs(dz) ? "x" : "z")
                                                      : (std::abs(dy) > std::abs(dz) ? "y" : "z");
        result.axis     = axis[0];
        result.value    = axis == "x" ? reg.pointX : (axis == "y" ? reg.pointY : reg.pointZ);
        result.boundary = (axis == "x" ? reg.centerX : (axis == "y" ? reg.centerY : reg.centerZ))
                        + ratio * (axis == "x" ? dx : (axis == "y" ? dy : dz));
    }
    return result;
}
// 2D/3D Square
PLAPI inline Boundary getBoundary(const Square2& reg) {
    Boundary result;
    if (reg.pointX < reg.leftTopX || reg.pointX > reg.rightBottomX) {
        result.axis     = "x";
        result.value    = reg.pointX;
        result.boundary = reg.pointX < reg.leftTopX ? reg.leftTopX : reg.rightBottomX;
    } else if (reg.pointZ < reg.leftTopZ || reg.pointZ > reg.rightBottomZ) {
        result.axis     = "z";
        result.value    = reg.pointZ;
        result.boundary = reg.pointZ < reg.leftTopZ ? reg.leftTopZ : reg.rightBottomZ;
    }
    return result;
}
PLAPI inline Boundary getBoundary(const Square3& reg) {
    Boundary result;
    if (reg.pointX < reg.leftTopX || reg.pointX > reg.rightBottomX) {
        result.axis     = 'x';
        result.value    = reg.pointX;
        result.boundary = reg.pointX < reg.leftTopX ? reg.leftTopX : reg.rightBottomX;
    } else if (reg.pointY < reg.leftTopY || reg.pointY > reg.rightBottomY) {
        result.axis     = 'y';
        result.value    = reg.pointY;
        result.boundary = reg.pointY < reg.leftTopY ? reg.leftTopY : reg.rightBottomY;
    } else if (reg.pointZ < reg.leftTopZ || reg.pointZ > reg.rightBottomZ) {
        result.axis     = 'z';
        result.value    = reg.pointZ;
        result.boundary = reg.pointZ < reg.leftTopZ ? reg.leftTopZ : reg.rightBottomZ;
    }
    return result;
}
// 2D/3D CenteredSquare
PLAPI inline Boundary getBoundary(const CenteredSquare2& reg) {
    Boundary result;
    if (std::abs(reg.pointX - reg.centerX) > reg.width) {
        result.axis     = "x";
        result.value    = reg.pointX;
        result.boundary = reg.pointX < reg.centerX ? reg.centerX - reg.width : reg.centerX + reg.width;
    } else if (std::abs(reg.pointZ - reg.centerZ) > reg.width) {
        result.axis     = "z";
        result.value    = reg.pointZ;
        result.boundary = reg.pointZ < reg.centerZ ? reg.centerZ - reg.width : reg.centerZ + reg.width;
    }
    return result;
}
PLAPI inline Boundary getBoundary(const CenteredSquare3& reg) {
    Boundary result;
    if (std::abs(reg.pointX - reg.centerX) > reg.width) {
        result.axis     = 'x';
        result.value    = reg.pointX;
        result.boundary = reg.pointX < reg.centerX ? reg.centerX - reg.width : reg.centerX + reg.width;
    } else if (std::abs(reg.pointY - reg.centerY) > reg.width) {
        result.axis     = 'y';
        result.value    = reg.pointY;
        result.boundary = reg.pointY < reg.centerY ? reg.centerY - reg.width : reg.centerY + reg.width;
    } else if (std::abs(reg.pointZ - reg.centerZ) > reg.width) {
        result.axis     = 'z';
        result.value    = reg.pointZ;
        result.boundary = reg.pointZ < reg.centerZ ? reg.centerZ - reg.width : reg.centerZ + reg.width;
    }
    return result;
}


PLAPI inline int _randomInt(int min, int max) {
    std::random_device rd;
    auto               seed_data = rd()
                   ^ (std::hash<long long>()(std::chrono::high_resolution_clock::now().time_since_epoch().count())
                      + std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937_64                    mt(seed_data);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(mt);
}

struct RCircle {
    double centerX, width, centerZ;
};
struct RSquare {
    double leftTopX, leftTopZ, rightBottomX, rightBottomZ;
};
struct RCenteredSquare {
    double centerX, width, centerZ;
};

// 2D Circle      x    z
PLAPI inline std::pair<int, int> randomXZ(const RCircle& reg) {
    double minX = reg.centerX - reg.width;
    double maxX = reg.centerX + reg.width;
    double minZ = reg.centerZ - reg.width;
    double maxZ = reg.centerZ + reg.width;
    return {_randomInt(minX, maxX), _randomInt(minZ, maxZ)};
}
// 2D Square
PLAPI inline std::pair<int, int> randomXZ(const RSquare& reg) {
    RSquare reg1 = reg;
    if (reg1.leftTopX > reg1.rightBottomX) std::swap(reg1.leftTopX, reg1.rightBottomX);
    if (reg1.leftTopZ > reg1.rightBottomZ) std::swap(reg1.leftTopZ, reg1.rightBottomZ);
    return {_randomInt(reg1.leftTopX, reg1.rightBottomX), _randomInt(reg1.leftTopZ, reg1.rightBottomZ)};
}
// 2D CenteredSquare
PLAPI inline std::pair<int, int> randomXZ(const RCenteredSquare& reg) {
    double minX = reg.centerX - reg.width;
    double maxX = reg.centerX + reg.width;
    double minZ = reg.centerZ - reg.width;
    double maxZ = reg.centerZ + reg.width;
    if (minX > maxX) std::swap(minX, maxX);
    if (minZ > maxZ) std::swap(minZ, maxZ);
    return {_randomInt(minX, maxX), _randomInt(minZ, maxZ)};
}


struct FindResult {
    int  x, y, z;
    int  dimid;
    bool status = false;

    FindResult() {}
    FindResult(int x, int z, int dim) : x(x), z(z), dimid(dim) {}
};


PLAPI inline FindResult findSafePos(
    int const                  findX,                 // 要查找的x
    int const                  findZ,                 // 要查找的z
    int const                  findDimid,             // 要查找的维度
    int const                  startY          = 320, // 从这个高度开始遍历
    int const                  stopY           = -64, // 停止遍历的高度
    std::vector<string> const& dangerousBlocks = {"minecraft:lava", "minecraft:flowing_lava"}, // 危险方块
    int const                  offset1         = 1,                                            // 偏移量1
    int const                  offset2         = 2                                             // 偏移量2
) {
    FindResult result(findX, findZ, findDimid);
    try {
        if (startY <= stopY) return result; // 起始高度大于或等于停止高度，直接返回

        BlockPos          bp(findX, startY, findZ);
        static const auto getBlock = [&](int y) {
            auto bp2 = bp;
            bp2.y    = y;
            return mc::getBlock(bp2, findDimid);
        };

        int currentTraversalY = startY; // 当前遍历的y值
        while (currentTraversalY > stopY) {
            try {
                bp.y           = currentTraversalY; // 更新BlockPos对象的y值以匹配当前的currentTraversalY
                auto const& bl = mc::getBlock(bp, findDimid); // 获取方块对象引用

                if (bl.isAir()) {
                    currentTraversalY--; // 空气方块跳过
                    continue;
                } else if (currentTraversalY <= stopY || utils::some(dangerousBlocks, bl.getTypeName())) {
                    break;
                } else if (!bl.isAir() &&                                   // 落脚方块
                           getBlock(currentTraversalY + offset1).isAir()    // 玩家身体 下半
                           && getBlock(currentTraversalY + offset2).isAir() // 玩家身体 上半
                ) {
                    // 安全位置   落脚点安全、上两格是空气
                    result.y      = currentTraversalY + 1; // 往上跳一格
                    result.status = true;
                    break;
                }
                currentTraversalY--;
            } catch (...) {
                return result;
            }
        }
        return result;
    } catch (...) {
        return result;
    }
}


} // namespace plot::area