#include "plotcraft/core/PlotPos.h"
#include "fmt/format.h"
#include "plotcraft/Config.h"
#include "plotcraft/core/TemplateManager.h"
#include <cmath>


namespace plo {
using TemplateManager = core::TemplateManager;


/*
    基准点：Vec3{0,0,0}
    地皮(0,0).minPos = Vec3{0,-64,0}
    地皮(0,0).maxPos = Vec3{0 + config::cfg.generator.plotWidth -1 ,320,0 + config::cfg.generator.plotWidth -1}
 */

PlotPos::PlotPos() : mX(0), mZ(0) {}
PlotPos::PlotPos(int x, int z) : mX(x), mZ(z) {
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

PlotPos::PlotPos(const Vec3& vec3) {
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
int PlotPos::getSurfaceY() const {
    return TemplateManager::isUseTemplate() ? (TemplateManager::mTemplateData.template_offset + 1)
                                            : -64 + (config::cfg.generator.subChunkNum * 16);
}
bool   PlotPos::isValid() const { return !mVertexs.empty(); }
string PlotPos::getPlotID() const { return fmt::format("({0},{1})", mX, mZ); }
Vec3   PlotPos::getSafestPos() const {
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

std::vector<PlotPos> PlotPos::getAdjacentPlots() const {
    return {PlotPos(mX - 1, mZ), PlotPos(mX + 1, mZ), PlotPos(mX, mZ - 1), PlotPos(mX, mZ + 1)};
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
// 判断点是否在多边形内部（射线法）
bool PlotPos::isPointInPolygon(const Vec3& point, const std::vector<Vec3>& polygon) {
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


} // namespace plo