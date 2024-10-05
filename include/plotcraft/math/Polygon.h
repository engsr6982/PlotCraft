#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "plotcraft/Global.h"


namespace plot {


/**
 * @brief 多边形几何类，支持凸多边形(静态类)
 */
class Polygon {
public:
    Polygon()                          = delete;
    ~Polygon()                         = delete;
    Polygon(Polygon&&)                 = delete;
    Polygon(Polygon const&)            = delete;
    Polygon& operator=(Polygon&&)      = delete;
    Polygon& operator=(Polygon const&) = delete;

    /**
     * @brief 判断点是否在多边形边上
     */
    static bool isOnEdge(Vertexs const& polygon, Vec3 const& point);

    /**
     * @brief 判断AABB区域是否在多边形边上
     */
    static bool isAABBOnEdge(Vertexs const& polygon, BlockPos const& min, BlockPos const& max);

    /**
     * @brief 判断一个圆是否在多边形边上
     */
    static bool isCircleOnEdge(Vertexs const& polygon, Vec3 const& center, float radius);

    /**
     * @brief 尝试合并两个多边形(凸包算法 + Graham扫描)
     * @param other 另一个多边形
     * @return 合并后的多边形，如果无法合并则返回空
     *
     * @warnning: 必须保证两个多边形是凸多边形，否则合并结果不正确
     */
    static Vertexs tryMerge(Vertexs const& mVertexs, Vertexs const& other);

    /**
     * @brief 判断点是否在多边形内(射线法)
     */
    static bool isPointInPolygon(Vertexs const& vertex, Vec3 const& point);
    /**
     * @brief 检查多边形是否为凸多边形
     */
    static bool isConvex(Vertexs const& vertexs);

    /**
     * @brief 检查多边形的边是否为水平或垂直
     */
    static bool isEdgeHorizontalOrVertical(Vertexs const& vertexs);

    /**
     * @brief 获取AABB区域的顶点
     */
    static Vertexs getAABBAroundVertexs(BlockPos const& min, BlockPos const& max);

    /**
     * @brief 点是否在AABB区域内
     */
    static bool isPointInAABB(Vec3 const& point, BlockPos const& min, BlockPos const& max, bool includeY = false);

    /**
     * @brief 修复AABB区域
     */
    static void fixAABB(BlockPos& min, BlockPos& max);
    static void fixAABB(Vec3& min, Vec3& max);

    /**
     * @brief 两个AABB区域是否碰撞
     */
    static bool isAABBCollision(BlockPos const& min1, BlockPos const& max1, BlockPos const& min2, BlockPos const& max2);
};

} // namespace plot