#include "plotcraft/core/PPos.h"
#include "fmt/format.h"
#include "plotcraft/Config.h"
#include "plotcraft/core/TemplateManager.h"
#include <cmath>
#include <utility>


namespace plo {
using TemplateManager = core::TemplateManager;

// !Class: PPos
PPos::PPos() : mX(0), mZ(0) {}
PPos::PPos(int x, int z) : mX(x), mZ(z) {
    auto& cfg = config::cfg.generator;

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

PPos::PPos(const Vec3& vec3) {
    auto& cfg = config::cfg.generator;

    // 计算总长度
    bool const isUseTemplate = TemplateManager::isUseTemplate();
    int const  roadWidth     = isUseTemplate ? TemplateManager::getCurrentTemplateRoadWidth() : cfg.roadWidth;
    int total  = isUseTemplate ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);
    int width  = isUseTemplate ? (total - (roadWidth * 2)) : cfg.plotWidth;
    int localX = static_cast<int>(std::floor(vec3.x)) % total;
    int localZ = static_cast<int>(std::floor(vec3.z)) % total;

    // 计算地皮坐标
    mX = std::floor(vec3.x / total);
    mZ = std::floor(vec3.z / total);

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
int PPos::getSurfaceY() const {
    return TemplateManager::isUseTemplate() ? (TemplateManager::mTemplateData.template_offset + 1)
                                            : -64 + (config::cfg.generator.subChunkNum * 16);
}
bool   PPos::isValid() const { return !mVertexs.empty(); }
string PPos::getPlotID() const { return fmt::format("({0},{1})", mX, mZ); }
Vec3   PPos::getSafestPos() const {
    if (isValid()) {
        auto& v3 = mVertexs[0];
        return Vec3{v3.x, getSurfaceY() + 1, v3.z};
    }
    return Vec3{};
}


bool PPos::isPosInPlot(const Vec3& vec3) const {
    if (vec3.y < -64 || vec3.y > 320) {
        return false;
    }
    return isPointInPolygon(vec3, mVertexs);
}

bool PPos::isPosOnBorder(const Vec3& vec3) const {
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


string PPos::toString() const {
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


bool PPos::operator!=(PPos const& other) const { return !(*this == other); }
bool PPos::operator==(PPos const& other) const { return other.mVertexs == this->mVertexs; }


// static
bool PPos::isAdjacent(const PPos& plot1, const PPos& plot2) {
    int dx = std::abs(plot1.mX - plot2.mX);
    int dz = std::abs(plot1.mZ - plot2.mZ);

    // 两个地皮相邻的条件:
    // 1. x坐标相同,z坐标相差1,或者
    // 2. z坐标相同,x坐标相差1
    // 3. 两个地皮都是有效的
    return ((dx == 0 && dz == 1) || (dx == 1 && dz == 0)) && (plot1.isValid() && plot2.isValid());
}
// 判断点是否在多边形内部（射线法）
bool PPos::isPointInPolygon(const Vec3& point, Vertexs const& polygon) {
    bool inside = false;
    int  n      = polygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].z <= point.z && point.z < polygon[j].z) || (polygon[j].z <= point.z && point.z < polygon[i].z))
            && (point.x < (polygon[j].x - polygon[i].x) * (point.z - polygon[i].z) / (polygon[j].z - polygon[i].z)
                              + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}


// !Class: Cube
Cube::Cube(BlockPos const& min, BlockPos const& max) : mMin(min), mMax(max) {
    if (mMin.x > mMax.x) std::swap(mMin.x, mMax.x);
    if (mMin.y > mMax.y) std::swap(mMin.y, mMax.y);
    if (mMin.z > mMax.z) std::swap(mMin.z, mMax.z);
}

Vertexs Cube::get2DVertexs() const {
    return {
        Vec3{mMin.x, 0, mMin.z}, // 左下
        Vec3{mMax.x, 0, mMin.z}, // 右下
        Vec3{mMax.x, 0, mMax.z}, // 右上
        Vec3{mMin.x, 0, mMax.z}, // 左上
        Vec3{mMin.x, 0, mMin.z}  // 回到起点，形成闭合多边形
    };
}

bool Cube::hasPos(BlockPos const& pos) const {
    return pos.x >= mMin.x && pos.x <= mMax.x && pos.y >= mMin.y && pos.y <= mMax.y && pos.z >= mMin.z
        && pos.z <= mMax.z;
}

std::vector<PPos> Cube::getRangedPlots() const {
    std::vector<PPos> rangedPlots;

    // 获取配置信息
    auto& cfg           = config::cfg.generator;
    bool  isUseTemplate = TemplateManager::isUseTemplate();
    int total = isUseTemplate ? (TemplateManager::getCurrentTemplateChunkNum() * 16) : (cfg.plotWidth + cfg.roadWidth);

    // 计算可能涉及的地皮范围
    int minPlotX = std::floor(static_cast<double>(mMin.x) / total);
    int maxPlotX = std::ceil(static_cast<double>(mMax.x) / total);
    int minPlotZ = std::floor(static_cast<double>(mMin.z) / total);
    int maxPlotZ = std::ceil(static_cast<double>(mMax.z) / total);

    // 遍历可能的地皮
    for (int x = minPlotX; x <= maxPlotX; ++x) {
        for (int z = minPlotZ; z <= maxPlotZ; ++z) {
            PPos plot(x, z);

            // 检查地皮是否与Cube有交集
            if (plot.isValid()) {
                bool hasIntersection = false;
                for (const auto& vertex : plot.mVertexs) {
                    if (vertex.x >= mMin.x && vertex.x <= mMax.x && vertex.z >= mMin.z && vertex.z <= mMax.z
                        && mMin.y <= 320 && mMax.y >= -64) {
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
bool Cube::operator==(const Cube& other) const { return mMin == other.mMin && mMax == other.mMax; };
bool Cube::operator!=(const Cube& other) const { return !(*this == other); };

// static
bool Cube::isCollision(Cube const& cube1, Cube const& cube2) {
    return !(
        cube1.mMax.x < cube2.mMin.x || cube1.mMin.x > cube2.mMax.x || cube1.mMax.y < cube2.mMin.y
        || cube1.mMin.y > cube2.mMax.y || cube1.mMax.z < cube2.mMin.z || cube1.mMin.z > cube2.mMax.z
    );
}


} // namespace plo