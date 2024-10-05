#include "plotcraft/math/Polygon.h"


namespace plot {


/**
 * @brief 判断点是否在多边形边上
 */
bool Polygon::isOnEdge(Vertexs const& polygon, Vec3 const& point) {
    for (size_t i = 0; i < polygon.size() - 1; ++i) {
        const Vec3& v1 = polygon[i];
        const Vec3& v2 = polygon[(i + 1) % polygon.size()];

        // 检查点是否在当前边上
        if ((point.x >= std::min(v1.x, v2.x) && point.x <= std::max(v1.x, v2.x))
            && (point.z >= std::min(v1.z, v2.z) && point.z <= std::max(v1.z, v2.z))) {
            // 如果边是垂直的
            if (v1.x == v2.x) {
                if (point.x == v1.x) return true;
            }
            // 如果边是水平的
            else if (v1.z == v2.z) {
                if (point.z == v1.z) return true;
            }
        }
    }
    return false;
}

/**
 * @brief 判断AABB区域是否在多边形边上
 */
bool Polygon::isAABBOnEdge(Vertexs const& polygon, BlockPos const& min, BlockPos const& max) {
    // 检查Cube是否完全在地皮外部或内部
    bool allInside  = true;
    bool allOutside = true;
    for (const auto& corner : Polygon::getAABBAroundVertexs(min, max)) {
        bool inside  = isPointInPolygon(polygon, corner);
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
    for (size_t i = 0; i < polygon.size() - 1; ++i) {
        const BlockPos& v1 = polygon[i];
        const BlockPos& v2 = polygon[i + 1];

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

/**
 * @brief 判断一个圆是否在多边形边上
 */
bool Polygon::isCircleOnEdge(Vertexs const& polygon, Vec3 const& center, float radius) {
    // 快速检查：如果圆心到多边形中心的距离大于半径加上多边形对角线的一半，则一定不相交
    Vec3   plotCenter = (polygon[0] + polygon[2]) * 0.5;
    double dx         = center.x - plotCenter.x;
    double dz         = center.z - plotCenter.z;
    double centerDist = std::sqrt(dx * dx + dz * dz);
    double plotRadius = (polygon[2] - polygon[0]).length() * 0.5;
    if (centerDist > radius + plotRadius) {
        return false;
    }

    // 检查圆是否完全包含多边形或完全在多边形外部
    bool allInside  = true;
    bool allOutside = true;
    for (const auto& vertex : polygon) {
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

    // 检查圆是否与多边形的边相交
    for (size_t i = 0; i < polygon.size() - 1; ++i) {
        const Vec3& v1 = polygon[i];
        const Vec3& v2 = polygon[i + 1];

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

/**
 * @brief 尝试合并两个多边形(凸包算法 + Graham扫描)
 * @param other 另一个多边形
 * @return 合并后的多边形，如果无法合并则返回空
 *
 * @warnning: 必须保证两个多边形是凸多边形，否则合并结果不正确
 */
Vertexs Polygon::tryMerge(Vertexs const& mVertexs, Vertexs const& other) {
    // 首先检查两个多边形是否都是凸多边形
    if (!isConvex(mVertexs) || !isConvex(other)) {
        return {};
    }

    // 合并两个多边形的顶点
    std::vector<Vec3> mergedVertexs = mVertexs;
    mergedVertexs.insert(mergedVertexs.end(), other.begin(), other.end());

    // 使用凸包算法获取合并后的顶点
    std::vector<Vec3> sortedVertexs = mergedVertexs;

    // 使用 Graham 扫描算法计算凸包
    std::sort(sortedVertexs.begin(), sortedVertexs.end(), [](const Vec3& a, const Vec3& b) -> bool {
        if (a.z != b.z) return a.z < b.z;
        return a.x < b.x;
    });

    Vec3 pivot = sortedVertexs[0];
    std::sort(sortedVertexs.begin() + 1, sortedVertexs.end(), [&](const Vec3& a, const Vec3& b) -> bool {
        double angleA = std::atan2(a.z - pivot.z, a.x - pivot.x);
        double angleB = std::atan2(b.z - pivot.z, b.x - pivot.x);
        if (angleA == angleB) return (pivot - a).length() < (pivot - b).length();
        return angleA < angleB;
    });

    std::vector<Vec3> hull;
    hull.push_back(sortedVertexs[0]);
    hull.push_back(sortedVertexs[1]);

    for (size_t i = 2; i < sortedVertexs.size(); ++i) {
        while (hull.size() >= 2) {
            Vec3   q     = hull[hull.size() - 2];
            Vec3   r     = hull[hull.size() - 1];
            Vec3   s     = sortedVertexs[i];
            double cross = (r.x - q.x) * (s.z - q.z) - (r.z - q.z) * (s.x - q.x);
            if (cross > 0) break;
            hull.pop_back();
        }
        hull.push_back(sortedVertexs[i]);
    }

    // 确保多边形闭合
    if (hull.front() != hull.back()) {
        hull.push_back(hull.front());
    }

    // 检查合并后的多边形是否有效
    if (hull.size() < 4) { // 至少三个顶点加闭合点
        return {};
    }

    // 检查是否存在斜边
    if (!isEdgeHorizontalOrVertical(hull)) {
        return {};
    }

    return hull;
}

/**
 * @brief 判断点是否在多边形内(射线法)
 */
bool Polygon::isPointInPolygon(Vertexs const& vertex, Vec3 const& point) {
    bool inside = false;
    int  n      = (int)vertex.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((vertex[i].z <= point.z && point.z < vertex[j].z) || (vertex[j].z <= point.z && point.z < vertex[i].z))
            && (point.x
                < (vertex[j].x - vertex[i].x) * (point.z - vertex[i].z) / (vertex[j].z - vertex[i].z) + vertex[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

/**
 * @brief 检查多边形是否为凸多边形
 */
bool Polygon::isConvex(Vertexs const& vertexs) {
    if (vertexs.size() < 4) { // 包括闭合点，至少需要4个点
        return false;
    }

    bool sign    = false;
    bool signSet = false;

    for (size_t i = 0; i < vertexs.size() - 1; ++i) {
        const Vec3& current   = vertexs[i];
        const Vec3& next      = vertexs[(i + 1) % (vertexs.size() - 1)];
        const Vec3& afterNext = vertexs[(i + 2) % (vertexs.size() - 1)];

        double dx1 = next.x - current.x;
        double dz1 = next.z - current.z;
        double dx2 = afterNext.x - next.x;
        double dz2 = afterNext.z - next.z;

        double crossProduct = dx1 * dz2 - dz1 * dx2;

        if (!signSet) {
            sign    = crossProduct > 0;
            signSet = true;
        } else if ((crossProduct > 0) != sign) {
            return false;
        }
    }

    return true;
}

/**
 * @brief 检查多边形的边是否为水平或垂直
 */
bool Polygon::isEdgeHorizontalOrVertical(Vertexs const& vertexs) {
    for (size_t i = 0; i < vertexs.size() - 1; ++i) {
        const Vec3& v1 = vertexs[i];
        const Vec3& v2 = vertexs[(i + 1) % vertexs.size()];
        if (v1.x != v2.x && v1.z != v2.z) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 获取AABB区域的顶点
 */
Vertexs Polygon::getAABBAroundVertexs(BlockPos const& min, BlockPos const& max) {
    return {
        Vec3{min.x, 0, min.z}, // 左下
        Vec3{max.x, 0, min.z}, // 右下
        Vec3{max.x, 0, max.z}, // 右上
        Vec3{min.x, 0, max.z}, // 左上
        Vec3{min.x, 0, min.z}  // 回到起点，形成闭合多边形
    };
}

/**
 * @brief 点是否在AABB区域内
 */
bool Polygon::isPointInAABB(Vec3 const& point, BlockPos const& min, BlockPos const& max, bool includeY) {
    return (point.x >= min.x && point.x <= max.x) && (point.z >= min.z && point.z <= max.z)
        && (includeY ? (point.y >= min.y && point.y <= max.y) : true);
}

/**
 * @brief 修复AABB区域
 */
void Polygon::fixAABB(BlockPos& min, BlockPos& max) {
    if (min.x > max.x) std::swap(min.x, max.x);
    if (min.z > max.z) std::swap(min.z, max.z);
    if (min.y > max.y) std::swap(min.y, max.y);
}
void Polygon::fixAABB(Vec3& min, Vec3& max) {
    if (min.x > max.x) std::swap(min.x, max.x);
    if (min.z > max.z) std::swap(min.z, max.z);
    if (min.y > max.y) std::swap(min.y, max.y);
}

/**
 * @brief 两个AABB区域是否碰撞
 */
bool Polygon::isAABBCollision(BlockPos const& min1, BlockPos const& max1, BlockPos const& min2, BlockPos const& max2) {
    return !(
        max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y || max1.z < min2.z || min1.z > max2.z
    );
}


} // namespace plot