#include "plotcraft/math/PlotPos.h"
#include "fmt/core.h"
#include "ll/api/service/Bedrock.h"
#include "mc/enums/BlockUpdateFlag.h"
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/dimension/Dimension.h"
#include "plotcraft/Config.h"
#include "plotcraft/Global.h"
#include "plotcraft/core/TemplateManager.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/math/PlotCross.h"
#include "plotcraft/math/PlotDirection.h"
#include "plotcraft/math/PlotRoad.h"
#include "plotcraft/math/Polygon.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>


namespace plot {


PlotPos::PlotPos(int x, int z) : mX(x), mZ(z) {
    if (data::PlotDBStorage::getInstance()._initClass(*this)) {
        return; // 从数据库中加载数据
    }

    auto& cfg = Config::cfg.generator;

    Vec3 min, max;

    // 计算地皮的四个顶点
    if (!core::TemplateManager::isUseTemplate()) {
        // DefaultGenerator
        int total = cfg.plotWidth + cfg.roadWidth;
        min       = Vec3{x * total, -64, z * total};
        max       = Vec3{min.x + cfg.plotWidth - 1, 320, min.z + cfg.plotWidth - 1};

    } else {
        // TemplateGenerator
        int r     = core::TemplateManager::getCurrentTemplateRoadWidth();
        int total = core::TemplateManager::getCurrentTemplateChunkNum() * 16;
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
    bool const isUseTemplate = core::TemplateManager::isUseTemplate();
    int const  roadWidth     = isUseTemplate ? core::TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int        total =
        isUseTemplate ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);
    int width  = isUseTemplate ? (total - (roadWidth * 2)) : cfg.plotWidth;
    int localX = static_cast<int>(std::floor(vec3.x)) % total;
    int localZ = static_cast<int>(std::floor(vec3.z)) % total;

    // 计算地皮坐标
    mX = (int)std::floor(vec3.x / total);
    mZ = (int)std::floor(vec3.z / total);


    Vec3 min, max;
    bool isValid = true;

    if (!core::TemplateManager::isUseTemplate()) {
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
std::unique_ptr<PlotPos> PlotPos::tryMerge(PlotPos const& other) const {
    if (!this->isValid() || !other.isValid()) {
        return nullptr;
    }

    // 检查两个地皮是否相邻
    if (!PlotPos::isAdjacent(*this, other)) {
        return nullptr;
    }
    Vertexs newRange = Polygon::tryMerge(mVertexs, other.mVertexs);
    if (newRange.empty()) {
        return nullptr;
    }
    auto ptr      = std::make_unique<PlotPos>();
    ptr->mVertexs = newRange;
    return ptr;
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
    return core::TemplateManager::isUseTemplate() ? (core::TemplateManager::mTemplateData.template_offset + 1)
                                                  : -64 + (Config::cfg.generator.subChunkNum * 16);
}
Vertexs PlotPos::getAABBVertexs(BlockPos const& min, BlockPos const& max) {
    return Polygon::getAABBAroundVertexs(min, max);
}
std::vector<PlotPos> PlotPos::getPlotPosAt(BlockPos const& min, BlockPos const& max) {
    std::vector<PlotPos> rangedPlots;

    // 获取配置信息
    auto& cfg           = Config::cfg.generator;
    bool  isUseTemplate = core::TemplateManager::isUseTemplate();
    int   total =
        isUseTemplate ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

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
    bool  isUseTemplate = core::TemplateManager::isUseTemplate();
    int   total =
        isUseTemplate ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

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


} // namespace plot