#include "plotcraft/math/PlotPos.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/enums/BlockUpdateFlag.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/dimension/Dimension.h"
#include "plotcraft/Config.h"
#include "plotcraft/Global.h"
#include "plotcraft/core/TemplateManager.h"
#include "plotcraft/data/PlotDBStorage.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>


namespace plo {
using TemplateManager = core::TemplateManager;


// !Class: PlotPos
PlotPos::PlotPos(int x, int z) : mX(x), mZ(z) {
    auto& cfg = Config::cfg.generator;

    Vec3 min, max;

    // 计算地皮的四个顶点
    if (!TemplateManager::isUseTemplate()) {
        // DefaultGenerator
        int total = cfg.plotWidth + cfg.roadWidth;
        min       = Vec3{x * total, -64, z * total};
        max       = Vec3{min.x + cfg.plotWidth - 1, 320, min.z + cfg.plotWidth - 1};

    } else {
        // TemplateGenerator
        int r     = TemplateManager::getCurrentTemplateRoadWidth();
        int total = TemplateManager::getCurrentTemplateChunkNum() * 16;
        min       = Vec3{x * total + r, -64, z * total + r};
        max       = Vec3{min.x + total - r, 320, min.z + total - r};
    }

    // 按顺时针顺序存储顶点
    mVertexs = {
        min, // 左下角
        Vec3{max.x, min.y, min.z}, // 右下角
        max, // 右上角
        Vec3{min.x, min.y, max.z}, // 左上角
        min  // 回到起点，形成闭合多边形
    };
}
PlotPos::PlotPos(const Vec3& vec3) {
    auto& cfg = Config::cfg.generator;

    // 计算总长度
    bool const isUseTemplate = TemplateManager::isUseTemplate();
    int const  roadWidth     = isUseTemplate ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int total  = isUseTemplate ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);
    int width  = isUseTemplate ? (total - (roadWidth * 2)) : cfg.plotWidth;
    int localX = static_cast<int>(std::floor(vec3.x)) % total;
    int localZ = static_cast<int>(std::floor(vec3.z)) % total;

    // 计算地皮坐标
    mX = (int)std::floor(vec3.x / total);
    mZ = (int)std::floor(vec3.z / total);

    Vec3 min, max;
    bool isValid = true;

    if (!TemplateManager::isUseTemplate()) {
        // DefaultGenerator
        if (localX < 0) localX += total;
        if (localZ < 0) localZ += total;
        if (localX >= width || localZ >= width) {
            isValid = false;
        } else {
            min = Vec3{mX * total, -64, mZ * total};
            max = Vec3{min.x + width - 1, 320, min.z + width - 1};
        }

    } else {
        // TemplateGenerator
        if (localX < 1) localX += total;
        if (localZ < 1) localZ += total;
        if (localX > width || localZ > width) {
            isValid = false;
        } else {
            min = Vec3{mX * total + roadWidth, -64, mZ * total + roadWidth};
            max = Vec3{min.x + width - roadWidth, 320, min.z + width - roadWidth};
        }
    }


    if (isValid) {
        // 按顺时针顺序存储顶点
        mVertexs = {
            min, // 左下角
            Vec3{max.x, min.y, min.z}, // 右下角
            max, // 右上角
            Vec3{min.x, min.y, max.z}, // 左上角
            min  // 回到起点，形成闭合多边形
        };
    } else {
        mVertexs.clear();
        mX = 0;
        mZ = 0;
    }
}

int PlotPos::getSurfaceY() const { return PlotPos::getSurfaceYStatic(); }

bool PlotPos::isValid() const { return !mVertexs.empty(); }

string PlotPos::getPlotID() const { return fmt::format("({0},{1})", mX, mZ); }

Vec3 PlotPos::getSafestPos() const {
    if (isValid()) {
        auto& v3 = mVertexs[0];
        return Vec3{v3.x, getSurfaceY() + 1, v3.z};
    }
    return Vec3{};
}

bool PlotPos::isPosInPlot(const Vec3& vec3) const {
    if (vec3.y < -64 || vec3.y > 320) {
        return false;
    }
    return isPointInPolygon(vec3, mVertexs);
}

bool PlotPos::isPosOnBorder(const Vec3& vec3) const {
    if (vec3.y < -64 || vec3.y > 320) {
        return false;
    }

    // 检查点是否在任何边上
    for (size_t i = 0; i < mVertexs.size() - 1; ++i) {
        const Vec3& v1 = mVertexs[i];
        const Vec3& v2 = mVertexs[i + 1];

        // 检查点是否在当前边上
        if ((vec3.x >= std::min(v1.x, v2.x) && vec3.x <= std::max(v1.x, v2.x))
            && (vec3.z >= std::min(v1.z, v2.z) && vec3.z <= std::max(v1.z, v2.z))) {
            // 如果边是垂直的
            if (v1.x == v2.x) {
                if (vec3.x == v1.x) return true;
            }
            // 如果边是水平的
            else if (v1.z == v2.z) {
                if (vec3.z == v1.z) return true;
            }
            // 如果边是斜的（虽然在这个情况下不太可能）
            // else {
            //     double slope     = (v2.z - v1.z) / (v2.x - v1.x);
            //     double intercept = v1.z - slope * v1.x;
            //     if (std::abs(vec3.z - (slope * vec3.x + intercept)) < 1e-6) return true;
            // }
        }
    }

    return false;
}

bool PlotPos::isAABBOnBorder(BlockPos const& min, BlockPos const& max) const {
    if (!isValid()) {
        return false;
    }

    // 检查Cube是否完全在地皮外部或内部
    bool allInside  = true;
    bool allOutside = true;
    for (const auto& corner : PlotPos::getAABBVertexs(min, max)) {
        bool inside  = isPosInPlot(corner);
        allInside   &= inside;
        allOutside  &= !inside;
        if (!allInside && !allOutside) {
            break;
        }
    }
    if (allInside || allOutside) {
        return false;
    }

    // 检查Cube的边是否与地皮边界相交
    for (size_t i = 0; i < mVertexs.size() - 1; ++i) {
        const BlockPos& v1 = mVertexs[i];
        const BlockPos& v2 = mVertexs[i + 1];

        // 检查水平边
        if (v1.z == v2.z) {
            if (min.z <= v1.z && max.z >= v1.z
                && std::max(min.x, std::min(v1.x, v2.x)) <= std::min(max.x, std::max(v1.x, v2.x))) {
                return true;
            }
        }
        // 检查垂直边
        else if (v1.x == v2.x) {
            if (min.x <= v1.x && max.x >= v1.x
                && std::max(min.z, std::min(v1.z, v2.z)) <= std::min(max.z, std::max(v1.z, v2.z))) {
                return true;
            }
        }
    }

    return false;
}

bool PlotPos::isRadiusOnBorder(BlockPos const& center, int radius) const {
    if (!isValid()) {
        return false;
    }

    // 快速检查：如果圆心到地皮中心的距离大于半径加上地皮对角线的一半，则一定不相交
    Vec3   plotCenter = (mVertexs[0] + mVertexs[2]) * 0.5;
    double dx         = center.x - plotCenter.x;
    double dz         = center.z - plotCenter.z;
    double centerDist = std::sqrt(dx * dx + dz * dz);
    double plotRadius = (mVertexs[2] - mVertexs[0]).length() * 0.5;
    if (centerDist > radius + plotRadius) {
        return false;
    }

    // 检查圆是否完全包含地皮或完全在地皮外部
    bool allInside  = true;
    bool allOutside = true;
    for (const auto& vertex : mVertexs) {
        dx                 = vertex.x - center.x;
        dz                 = vertex.z - center.z;
        double distSquared = dx * dx + dz * dz;
        if (distSquared <= radius * radius) {
            allOutside = false;
        } else {
            allInside = false;
        }
        if (!allInside && !allOutside) {
            break;
        }
    }
    if (allInside || allOutside) {
        return false;
    }

    // 检查圆是否与地皮的边相交
    for (size_t i = 0; i < mVertexs.size() - 1; ++i) {
        const Vec3& v1 = mVertexs[i];
        const Vec3& v2 = mVertexs[i + 1];

        // 计算边的方向向量
        double edgeX = v2.x - v1.x;
        double edgeZ = v2.z - v1.z;

        // 计算从圆心到边起点的向量
        double vecX = center.x - v1.x;
        double vecZ = center.z - v1.z;

        // 计算边的长度的平方
        double edgeLengthSquared = edgeX * edgeX + edgeZ * edgeZ;

        // 计算圆心到边的投影长度比例
        double t = (vecX * edgeX + vecZ * edgeZ) / edgeLengthSquared;
        t        = std::max(0.0, std::min(1.0, t));

        // 计算圆心到边的最近点
        double nearestX = v1.x + t * edgeX;
        double nearestZ = v1.z + t * edgeZ;

        // 计算圆心到最近点的距离
        double distX       = center.x - nearestX;
        double distZ       = center.z - nearestZ;
        double distSquared = distX * distX + distZ * distZ;

        // 如果距离小于等于半径，则相交
        if (distSquared <= radius * radius) {
            return true;
        }
    }

    return false;
}

string PlotPos::toString() const {
#if !defined(DEBUG)
    return fmt::format("{0} | Vertex: {1}", getPlotID(), mVertexs.size());
#else
    string dbg;
    size_t i = 0;
    for (auto& v : mVertexs) {
        dbg += fmt::format("[{0}] => {1}\n", i++, v.toString());
    }
    return fmt::format("{0} | Vertex: {1}\n{2}", getPlotID(), mVertexs.size(), dbg);
#endif
}

bool PlotPos::operator!=(PlotPos const& other) const { return !(*this == other); }
bool PlotPos::operator==(PlotPos const& other) const { return other.mVertexs == this->mVertexs; }

// static
bool PlotPos::isAdjacent(const PlotPos& plot1, const PlotPos& plot2) {
    int dx = std::abs(plot1.mX - plot2.mX);
    int dz = std::abs(plot1.mZ - plot2.mZ);

    // 两个地皮相邻的条件:
    // 1. x坐标相同,z坐标相差1,或者
    // 2. z坐标相同,x坐标相差1
    // 3. 两个地皮都是有效的
    return ((dx == 0 && dz == 1) || (dx == 1 && dz == 0)) && (plot1.isValid() && plot2.isValid());
}
bool PlotPos::isPointInPolygon(const Vec3& point, Vertexs const& polygon) {
    // 判断点是否在多边形内部（射线法）
    bool inside = false;
    int  n      = (int)polygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].z <= point.z && point.z < polygon[j].z) || (polygon[j].z <= point.z && point.z < polygon[i].z))
            && (point.x < (polygon[j].x - polygon[i].x) * (point.z - polygon[i].z) / (polygon[j].z - polygon[i].z)
                              + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}
int PlotPos::getSurfaceYStatic() {
    return TemplateManager::isUseTemplate() ? (TemplateManager::mTemplateData.template_offset + 1)
                                            : -64 + (Config::cfg.generator.subChunkNum * 16);
}
Vertexs PlotPos::getAABBVertexs(BlockPos const& min, BlockPos const& max) {
    return {
        Vec3{min.x, 0, min.z}, // 左下
        Vec3{max.x, 0, min.z}, // 右下
        Vec3{max.x, 0, max.z}, // 右上
        Vec3{min.x, 0, max.z}, // 左上
        Vec3{min.x, 0, min.z}  // 回到起点，形成闭合多边形
    };
}
std::vector<PlotPos> PlotPos::getPlotPosAt(BlockPos const& min, BlockPos const& max) {
    std::vector<PlotPos> rangedPlots;

    // 获取配置信息
    auto& cfg           = Config::cfg.generator;
    bool  isUseTemplate = TemplateManager::isUseTemplate();
    int total = isUseTemplate ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    // 计算可能涉及的地皮范围
    int minPlotX = (int)std::floor(static_cast<double>(min.x) / total);
    int maxPlotX = (int)std::ceil(static_cast<double>(max.x) / total);
    int minPlotZ = (int)std::floor(static_cast<double>(min.z) / total);
    int maxPlotZ = (int)std::ceil(static_cast<double>(max.z) / total);

    // 遍历可能的地皮
    for (int x = minPlotX; x <= maxPlotX; ++x) {
        for (int z = minPlotZ; z <= maxPlotZ; ++z) {
            PlotPos plot(x, z);

            // 检查地皮是否与Cube有交集
            if (plot.isValid()) {
                bool hasIntersection = false;
                for (const auto& vertex : plot.mVertexs) {
                    if (vertex.x >= min.x && vertex.x <= max.x && vertex.z >= min.z && vertex.z <= max.z && min.y <= 320
                        && max.y >= -64) {
                        hasIntersection = true;
                        break;
                    }
                }

                if (hasIntersection) {
                    rangedPlots.push_back(plot);
                }
            }
        }
    }

    return rangedPlots;
}
std::vector<PlotPos> PlotPos::getPlotPosAt(BlockPos const& center, int radius) {
    std::vector<PlotPos> rangedPlots;

    // 获取配置信息
    auto& cfg           = Config::cfg.generator;
    bool  isUseTemplate = TemplateManager::isUseTemplate();
    int total = isUseTemplate ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    // 计算可能涉及的地皮范围
    int minPlotX = (int)std::floor((center.x - radius) / static_cast<double>(total));
    int maxPlotX = (int)std::ceil((center.x + radius) / static_cast<double>(total));
    int minPlotZ = (int)std::floor((center.z - radius) / static_cast<double>(total));
    int maxPlotZ = (int)std::ceil((center.z + radius) / static_cast<double>(total));

    // 遍历可能的地皮
    for (int x = minPlotX; x <= maxPlotX; ++x) {
        for (int z = minPlotZ; z <= maxPlotZ; ++z) {
            PlotPos plot(x, z);

            // 检查地皮是否与半径相交
            if (plot.isValid()) {
                bool hasIntersection = false;
                for (const auto& vertex : plot.mVertexs) {
                    double dx = vertex.x - center.x;
                    double dz = vertex.z - center.z;
                    if (dx * dx + dz * dz <= radius * radius) {
                        hasIntersection = true;
                        break;
                    }
                }

                if (hasIntersection) {
                    rangedPlots.push_back(plot);
                }
            }
        }
    }

    return rangedPlots;
}
bool PlotPos::isAABBCollision(BlockPos const& min1, BlockPos const& max1, BlockPos const& min2, BlockPos const& max2) {
    return !(
        max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y || max1.z < min2.z || min1.z > max2.z
    );
}


void PlotPos::fillBorder(Block const& block, PlotDirection direction) {
    if (direction == PlotDirection::NE || direction == PlotDirection::NW || direction == PlotDirection::SE
        || direction == PlotDirection::SW || direction == PlotDirection::Unknown) {
        throw std::runtime_error("PlotPos::fillBorder: Invalid direction");
    }
    if (!isValid()) {
        throw std::runtime_error("PlotPos::fillBorder: Invalid plot");
    }
}
void PlotPos::fixBorder(PlotDirection direction) {
    Block const& block = Block::tryGetFromRegistry(Config::cfg.generator.borderBlock);
    fillBorder(block, direction);
}


// !Class: PlotCross
PlotCross::PlotCross(int x, int z) : mX(x), mZ(z) {
    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto& min = mDiagonPos.first;
    auto& max = mDiagonPos.second;

    bool const temp  = TemplateManager::isUseTemplate();
    int const  road  = temp ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width = temp ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    min.x = mX * width + width - road;
    min.z = mZ * width + width - road;

    max.x = min.x + road - 1;
    max.z = min.z + road - 1;

    min.y  = -64;
    max.y  = 320;
    mValid = true;
}
PlotCross::PlotCross(Vec3 const& vec3) {
    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto& min = mDiagonPos.first;
    auto& max = mDiagonPos.second;

    bool const temp  = TemplateManager::isUseTemplate();
    int const  road  = temp ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width = temp ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    // 计算全局坐标在哪个大区域内
    mX = (int)std::floor(vec3.x / width);
    mZ = (int)std::floor(vec3.z / width);

    // 计算在大区域内的局部坐标
    int localX = (int)std::floor(vec3.x) - (mX * width);
    int localZ = (int)std::floor(vec3.z) - (mZ * width);

    bool isValid = true;
    if (!temp) {
        // DefaultGenerator
        if (localX < cfg.plotWidth || localX > width - 1 || localZ < cfg.plotWidth || localZ > width - 1) {
            isValid = false;
        }
    } else {
        // TemplateGenerator
        throw std::runtime_error("TemplateGenerator not implemented yet");
    }

    if (isValid) {
        min.x = mX * width + width - road;
        min.z = mZ * width + width - road;

        max.x = min.x + road - 1;
        max.z = min.z + road - 1;

        min.y = -64;
        max.y = 320;
    } else {
        mX  = 0;
        mZ  = 0;
        min = Vec3{0, 0, 0};
        max = Vec3{0, 0, 0};
    }
    mValid = isValid;
}

bool    PlotCross::isValid() const { return mValid; }
CrossID PlotCross::getCrossID() const { return fmt::format("({}, {})", mX, mZ); }
string  PlotCross::toString() const {
    return fmt::format("{} | {} => {}", getCrossID(), mDiagonPos.first.toString(), mDiagonPos.second.toString());
}
bool PlotCross::hasPoint(BlockPos const& pos) const {
    int const&  x   = pos.x;
    int const&  z   = pos.z;
    auto const& min = mDiagonPos.first;
    auto const& max = mDiagonPos.second;
    return x >= min.x && x <= max.x && z >= min.z && z <= max.z;
}
void PlotCross::fill(Block const& block, bool removeBorder) {
    auto min = removeBorder ? mDiagonPos.first - 1 : mDiagonPos.first;
    auto max = removeBorder ? mDiagonPos.second + 1 : mDiagonPos.second;

    auto dim = ll::service::getLevel()->getDimension(getPlotWorldDimensionId());
    if (!dim) {
        return;
    }

    auto&        bs  = dim->getBlockSourceFromMainChunkSource();
    Block const& air = Block::tryGetFromRegistry("minecraft:air").value();
    int const    y   = PlotPos::getSurfaceYStatic() - 1;

    for (int x = min.x; x <= max.x; x++) {
        for (int z = min.z; z <= max.z; z++) {
            auto& bl = bs.getBlock(x, y, z);

            if (bl.isAir()) {
                bs.setBlock(x, y, z, block, (int)BlockUpdateFlag::AllPriority, nullptr);
            }

            if (removeBorder
                && ((x == min.x && z == min.z) || (x == min.x && z == max.z) || (x == max.x && z == min.z)
                    || (x == max.x && z == max.z))) {
                bs.setBlock(x, y + 1, z, air, (int)BlockUpdateFlag::AllPriority, nullptr); // 替换四个角的方块为空气
            }
        }
    }
}


// !Class: PlotRoad
PlotRoad::PlotRoad(int x, int z, PlotDirection direction) : mX(x), mZ(z) {
    if (direction != PlotDirection::East && direction != PlotDirection::South) {
        throw std::runtime_error("PlotRoad::PlotRoad: Invalid direction");
    }

    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto&      min   = mDiagonPos.first;
    auto&      max   = mDiagonPos.second;
    bool const temp  = TemplateManager::isUseTemplate();
    int const  road  = temp ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width = temp ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);
    int        plot_size = width - road;

    if (direction == PlotDirection::East) {
        // x+
        min.x = mX * width + plot_size;
        max.x = min.x + road - 1;
        min.z = mZ * width;
        max.z = min.z + plot_size - 1;

    } else {
        // z+
        min.x = mX * width;
        max.x = min.x + plot_size - 1;
        min.z = mZ * width + plot_size;
        max.z = min.z + road - 1;
    }
    mDirection = direction;
}

PlotRoad::PlotRoad(Vec3 const& vec3) {
    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto& min = mDiagonPos.first;
    auto& max = mDiagonPos.second;
    min.y     = -64;
    max.y     = 320;

    bool const temp  = TemplateManager::isUseTemplate();
    int const  road  = temp ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width = temp ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    int plot_size = width - road;

    // 使用 floor 来处理负坐标
    mX = (int)std::floor(vec3.x / (double)width);
    mZ = (int)std::floor(vec3.z / (double)width);

    // 使用 fmod 来正确处理负坐标的本地坐标
    double localX = std::fmod(vec3.x, width);
    double localZ = std::fmod(vec3.z, width);
    if (localX < 0) localX += width;
    if (localZ < 0) localZ += width;

    mValid = true;

    // 判断是纵向道路还是横向道路
    if (localX >= plot_size && localX < width && localZ < plot_size) {
        // 纵向道路
        min.x      = mX * width + plot_size;
        max.x      = min.x + road - 1;
        min.z      = mZ * width;
        max.z      = min.z + plot_size - 1;
        mDirection = PlotDirection::East;

    } else if (localZ >= plot_size && localZ < width && localX < plot_size) {
        // 横向道路
        min.x      = mX * width;
        max.x      = min.x + plot_size - 1;
        min.z      = mZ * width + plot_size;
        max.z      = min.z + road - 1;
        mDirection = PlotDirection::South;

    } else {
        mValid = false; // 不在道路上
        min    = Vec3{0, 0, 0};
        max    = Vec3{0, 0, 0};
        mX     = 0;
        mZ     = 0;
    }
}

bool   PlotRoad::isValid() const { return mValid; }
RoadID PlotRoad::getRoadID() const { return fmt::format("{}({}, {})", magic_enum::enum_name(mDirection), mX, mZ); }
string PlotRoad::toString() const {
    return fmt::format("{} | {} => {}", getRoadID(), mDiagonPos.first.toString(), mDiagonPos.second.toString());
}
bool PlotRoad::hasPoint(BlockPos const& pos) const {
    return pos.x >= mDiagonPos.first.x && pos.x <= mDiagonPos.second.x && pos.z >= mDiagonPos.first.z
        && pos.z <= mDiagonPos.second.z;
}
void PlotRoad::fill(Block const& block, bool removeBorder) {
    bool const isX = mDirection == PlotDirection::East;

    auto const& min = mDiagonPos.first;
    auto const& max = mDiagonPos.second;

    auto dim = ll::service::getLevel()->getDimension(getPlotWorldDimensionId());
    if (!dim) {
        return;
    }

    auto& bs  = dim->getBlockSourceFromMainChunkSource();
    auto& air = Block::tryGetFromRegistry("minecraft:air").value();

    int const y_border = PlotPos::getSurfaceYStatic();
    int const y_road   = y_border - 1;

    int min_x = min.x - 1;
    int max_x = max.x + 1;
    int min_z = min.z - 1;
    int max_z = max.z + 1;

    for (int x = min.x; x <= max.x; x++) {
        for (int z = min.z; z <= max.z; z++) {
            auto& bl = bs.getBlock(x, y_road, z);

            if (!bl.isAir()) {
                bs.setBlock(x, y_road, z, block, (int)BlockUpdateFlag::AllPriority, nullptr);
            }

            if (removeBorder) {
                if (isX && x - 1 == min_x) {
                    bs.setBlock(x - 1, y_border, z, air, (int)BlockUpdateFlag::AllPriority, nullptr);

                } else if (isX && x + 1 == max_x) {
                    bs.setBlock(x + 1, y_border, z, air, (int)BlockUpdateFlag::AllPriority, nullptr);

                } else if (!isX && z - 1 == min_z) {
                    bs.setBlock(x, y_border, z - 1, air, (int)BlockUpdateFlag::AllPriority, nullptr);

                } else if (!isX && z + 1 == max_z) {
                    bs.setBlock(x, y_border, z + 1, air, (int)BlockUpdateFlag::AllPriority, nullptr);
                }
            }
        }
    }
}
std::vector<PlotDirection> PlotRoad::getAfterFillingNeedFixBorderDirections() const {
    bool const isX = mDirection == PlotDirection::East;

    if (isX) {
        return {PlotDirection::East, PlotDirection::West};
    } else {
        return {PlotDirection::South, PlotDirection::North};
    }
}

} // namespace plo