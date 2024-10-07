#include "plotcraft/math/PlotCross.h"
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
#include "plotcraft/math/PlotDirection.h"
#include "plotcraft/math/PlotRoad.h"
#include "plotcraft/math/Polygon.h"
#include <algorithm>
#include <cmath>
#include <vector>


namespace plot {

PlotCross::PlotCross(int x, int z) : mX(x), mZ(z) {
    data::PlotDBStorage::getInstance()._initClass(*this);
    auto const& cfg = Config::cfg.generator;

    auto& min = mDiagonPos.first;
    auto& max = mDiagonPos.second;

    bool const temp = core::TemplateManager::isUseTemplate();
    int const  road = temp ? core::TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width =
        temp ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    min.x = (float)mX * width + width - road;
    min.z = (float)mZ * width + width - road;

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

    bool const temp = core::TemplateManager::isUseTemplate();
    int const  road = temp ? core::TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int const  width =
        temp ? (core::TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

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
        min.x = (float)mX * width + width - road;
        min.z = (float)mZ * width + width - road;

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

    for (int x = (int)min.x; x <= max.x; x++) {
        for (int z = (int)min.z; z <= max.z; z++) {
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


} // namespace plot