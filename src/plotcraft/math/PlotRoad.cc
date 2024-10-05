#include "plotcraft/math/PlotRoad.h"
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
#include <vector>


namespace plot {


PlotRoad::PlotRoad(int x, int z, PlotDirection direction) : mX(x), mZ(z) {
    if (direction != PlotDirection::East && direction != PlotDirection::South) {
        throw std::runtime_error("PlotRoad::PlotRoad: Invalid direction");
    }

    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto&      min  = mDiagonPos.first;
    auto&      max  = mDiagonPos.second;
    bool const temp = core::TemplateManager::isUseTemplate();
    int const  road = temp ? core::TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width =
        temp ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);
    int plot_size = width - road;

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

    bool const temp = core::TemplateManager::isUseTemplate();
    int const  road = temp ? core::TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width =
        temp ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

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


} // namespace plot