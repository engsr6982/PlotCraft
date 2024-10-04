#include "plotcraft/math/PlotPos.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/enums/BlockUpdateFlag.h"
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/dimension/Dimension.h"
#include "plotcraft/Config.h"
#include "plotcraft/Global.h"
#include "plotcraft/core/TemplateManager.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/math/Polygon.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>


namespace plo {
using TemplateManager = core::TemplateManager;


// !Class: PlotPos
PlotPos::PlotPos(int x, int z) : mX(x), mZ(z) {
    if (data::PlotDBStorage::getInstance()._initClass(*this)) {
        return; // 从数据库中加载数据
    }

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

    auto vtx = data::PlotDBStorage::getInstance()._getInitClassVertexs(*this);
    if (!vtx.empty() && Polygon::isPointInPolygon(vtx, vec3)) {
        this->mVertexs = vtx;

    } else if (isValid) {
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

int    PlotPos::getSurfaceY() const { return PlotPos::getSurfaceYStatic(); }
bool   PlotPos::isValid() const { return !mVertexs.empty(); }
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
    return Polygon::isOnEdge(mVertexs, vec3);
}
bool PlotPos::isAABBOnBorder(BlockPos const& min, BlockPos const& max) const {
    if (!isValid()) {
        return false;
    }
    return Polygon::isAABBOnEdge(mVertexs, min, max);
}
bool PlotPos::isRadiusOnBorder(BlockPos const& center, int radius) const {
    if (!isValid()) {
        return false;
    }
    return Polygon::isCircleOnEdge(mVertexs, center, radius);
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

void PlotPos::fixBorder() {
    if (!isValid()) {
        return;
    }
    Block const& block = Block::tryGetFromRegistry(Config::cfg.generator.borderBlock);
    int          y     = getSurfaceY();

    size_t   next = 0;
    BlockPos tmp1, tmp2;
    for (auto const& i : mVertexs) {
        next++;
        if (next == mVertexs.size()) {
            next = 0;
        }

        tmp1 = i;
        tmp2 = mVertexs[next];

        tmp1.y = y;
        tmp2.y = y;

        fillAABB(tmp1, tmp2, block);
    }
}
bool PlotPos::isAdjacent(PlotRoad const& road) const {
    if (!road.isValid()) {
        return false;
    }
    if (road.mDirection == PlotDirection::East) {
        return (road.mX == this->mX && road.mZ == this->mZ) || (road.mX + 1 == this->mX && road.mZ == this->mZ);
    } else {
        return (road.mX == this->mX && road.mZ == this->mZ) || (road.mX == this->mX && road.mZ + 1 == this->mZ);
    }
}
bool PlotPos::isCorner(PlotCross const& cross) const {
    if (!cross.isValid()) {
        return false;
    }
    return (cross.mX == this->mX && cross.mZ == this->mZ) ||         // 右上角 (0,0)
           (cross.mX == this->mX && cross.mZ + 1 == this->mZ) ||     // 左上角 (0,-1)
           (cross.mX + 1 == this->mX && cross.mZ + 1 == this->mZ) || // 左下角 (-1,-1)
           (cross.mX + 1 == this->mX && cross.mZ == this->mZ);       // 右下角 (-1,0)
}

std::vector<PlotPos> PlotPos::getRangedPlots() const {
    std::vector<PlotPos> rangedPlots;
    if (mVertexs.empty()) {
        return rangedPlots;
    }

    // 确定边界
    int minX = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int maxX = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int minZ = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    int maxZ = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    // 遍历边界内的所有地皮
    for (int x = minX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
         x <= maxX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
         ++x) {
        for (int z = minZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
             z <= maxZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
             ++z) {
            PlotPos plot(x, z);
            if (plot.isValid()) {
                // 检查地皮是否在多边形内部
                // 使用多边形包含判断
                for (const auto& vertex : plot.mVertexs) {
                    if (isPointInPolygon(vertex, mVertexs)) {
                        rangedPlots.emplace_back(plot);
                        break;
                    }
                }
            }
        }
    }

    return rangedPlots;
}
std::vector<PlotRoad> PlotPos::getRangedRoads() const {
    std::vector<PlotRoad> rangedRoads;
    if (mVertexs.empty()) {
        return rangedRoads;
    }

    // 确定边界
    int minX = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int maxX = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int minZ = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    int maxZ = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    // 遍历边界内的所有道路
    for (int x = minX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
         x <= maxX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
         ++x) {
        for (int z = minZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
             z <= maxZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
             ++z) {
            PlotRoad road(x, z, PlotDirection::East);
            if (road.isValid()
                && isPointInPolygon(Vec3{road.mDiagonPos.first.x, 0, road.mDiagonPos.first.z}, mVertexs)) {
                rangedRoads.emplace_back(road);
            }
            road = PlotRoad(x, z, PlotDirection::South);
            if (road.isValid()
                && isPointInPolygon(Vec3{road.mDiagonPos.first.x, 0, road.mDiagonPos.first.z}, mVertexs)) {
                rangedRoads.emplace_back(road);
            }
        }
    }

    return rangedRoads;
}
std::vector<PlotCross> PlotPos::getRangedCrosses() const {
    std::vector<PlotCross> rangedCrosses;
    if (mVertexs.empty()) {
        return rangedCrosses;
    }

    // 确定边界
    int minX = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int maxX = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.x < b.x;
               })->x;

    int minZ = std::min_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    int maxZ = std::max_element(mVertexs.begin(), mVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
                   return a.z < b.z;
               })->z;

    // 遍历边界内的所有路口
    for (int x = minX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
         x <= maxX / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
         ++x) {
        for (int z = minZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) - 1;
             z <= maxZ / (Config::cfg.generator.plotWidth + Config::cfg.generator.roadWidth) + 1;
             ++z) {
            PlotCross cross(x, z);
            if (cross.isValid()
                && isPointInPolygon(Vec3{cross.mDiagonPos.first.x, 0, cross.mDiagonPos.first.z}, mVertexs)) {
                rangedCrosses.emplace_back(cross);
            }
        }
    }

    return rangedCrosses;
}
std::optional<PlotPos> PlotPos::tryMerge(PlotPos const& other) {
    if (!this->isValid() || !other.isValid()) {
        return std::nullopt;
    }

    // 检查两个地皮是否相邻
    if (!PlotPos::isAdjacent(*this, other)) {
        return std::nullopt;
    }
    Vertexs newRange = Polygon::tryMerge(mVertexs, other.mVertexs);
    if (newRange.empty()) {
        return std::nullopt;
    }
    PlotPos res;
    res.mVertexs = newRange;
    return res;
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
    return Polygon::isPointInPolygon(polygon, point);
}
int PlotPos::getSurfaceYStatic() {
    return TemplateManager::isUseTemplate() ? (TemplateManager::mTemplateData.template_offset + 1)
                                            : -64 + (Config::cfg.generator.subChunkNum * 16);
}
Vertexs PlotPos::getAABBVertexs(BlockPos const& min, BlockPos const& max) {
    return Polygon::getAABBAroundVertexs(min, max);
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
    return Polygon::isAABBCollision(min1, max1, min2, max2);
}
void PlotPos::fillAABB(const BlockPos& min, const BlockPos& max, const Block& block) {
    auto& bs = ll::service::getLevel()->getDimension(getPlotWorldDimensionId())->getBlockSourceFromMainChunkSource();

    BlockPos start = min;
    BlockPos end   = max;
    Polygon::fixAABB(start, end);

    for (int x = start.x; x <= end.x; ++x) {
        for (int z = start.z; z <= end.z; ++z) {
            for (int y = start.y; y <= end.y; ++y) {
                bs.setBlock(x, y, z, block, (int)BlockUpdateFlag::AllPriority, nullptr);
            }
        }
    }
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
    return Polygon::isPointInAABB(pos, mDiagonPos.first, mDiagonPos.second);
}
void PlotCross::fill(Block const& block, bool removeBorder) {
    auto min = removeBorder ? mDiagonPos.first - 1 : mDiagonPos.first;
    auto max = removeBorder ? mDiagonPos.second + 1 : mDiagonPos.second;
    Polygon::fixAABB(min, max);

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

            if (!bl.isAir()) {
                bs.setBlock(x, y, z, block, (int)BlockUpdateFlag::AllPriority, nullptr);
            }

            auto& borderBlock = bs.getBlock(x, y + 1, z);
            if ((removeBorder && !borderBlock.isAir()) && (x == min.x || x == max.x || z == min.z || z == max.z)) {
                bs.setBlock(x, y + 1, z, air, (int)BlockUpdateFlag::AllPriority, nullptr);
            }
        }
    }
}
bool PlotCross::isAdjacent(PlotRoad const& road) const {
    if (!isValid() || !road.isValid()) {
        return false;
    }

    // 检查道路是否与路口相邻
    if (road.mDirection == PlotDirection::East) {
        return (road.mX == mX && road.mZ == mZ) || (road.mX == mX && road.mZ == mZ + 1);
    } else { // South
        return (road.mX == mX && road.mZ == mZ) || (road.mX == mX + 1 && road.mZ == mZ);
    }
}

std::vector<PlotRoad> PlotCross::getAdjacentRoads() const {
    std::vector<PlotRoad> roads;
    if (!isValid()) {
        return roads;
    }

    // 添加四个相邻道路
    roads.push_back(PlotRoad(mX, mZ, PlotDirection::East));
    roads.push_back(PlotRoad(mX, mZ + 1, PlotDirection::East));
    roads.push_back(PlotRoad(mX, mZ, PlotDirection::South));
    roads.push_back(PlotRoad(mX + 1, mZ, PlotDirection::South));

    // 移除无效的道路
    roads.erase(
        std::remove_if(roads.begin(), roads.end(), [](const PlotRoad& road) { return !road.isValid(); }),
        roads.end()
    );

    return roads;
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
    return Polygon::isPointInAABB(pos, mDiagonPos.first, mDiagonPos.second);
}
void PlotRoad::fill(Block const& block, bool removeBorder) {
    bool const isX = mDirection == PlotDirection::East;

    auto& min = mDiagonPos.first;
    auto& max = mDiagonPos.second;
    Polygon::fixAABB(min, max);

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
bool PlotRoad::isAdjacent(PlotCross const& cross) const {
    if (!isValid() || !cross.isValid()) {
        return false;
    }

    // 检查路口是否与道路相邻
    if (mDirection == PlotDirection::East) {
        return (cross.mX == mX && cross.mZ + 1 == mZ) || (cross.mX == mX && cross.mZ == mZ);
    } else { // South
        return (cross.mX + 1 == mX && cross.mZ == mZ) || (cross.mX == mX && cross.mZ == mZ);
    }
}

std::vector<PlotCross> PlotRoad::getAdjacentCrosses() const {
    std::vector<PlotCross> crosses;
    if (!isValid()) {
        return crosses;
    }

    // 添加两个相邻路口
    if (mDirection == PlotDirection::East) {
        crosses.push_back(PlotCross(mX, mZ - 1));
        crosses.push_back(PlotCross(mX, mZ));
    } else { // South
        crosses.push_back(PlotCross(mX - 1, mZ));
        crosses.push_back(PlotCross(mX, mZ));
    }

    // 移除无效的路口
    crosses.erase(
        std::remove_if(crosses.begin(), crosses.end(), [](const PlotCross& cross) { return !cross.isValid(); }),
        crosses.end()
    );

    return crosses;
}


} // namespace plo