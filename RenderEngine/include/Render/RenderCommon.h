#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

#include "GLRenderExport.h"
#include <QImage>
#include <functional>
#include <vector>
#include "Common/Brush.h"

namespace GLRhi
{
    // 截图数据结构体
    struct GLRENDER_EXPORT RenderSnap
    {
        QImage image;
        QRectF worldRect;
    };

    // 多段线
    // 使用内存对齐优化的线段数据结构
    struct GLRENDER_EXPORT PolylineData
    {
        long long id;                           // ID
        std::vector<float> verts{};             //  x, y, len
        std::vector<size_t> count;              // 线段顶点数
        Brush brush{ 0.0, 0.0, 0.0, 1.0, 0.0 }; // 渲染信息

        //// 构造函数确保数据一致性
        //PolylineData() = default;
        //
        //// 移动构造函数，优化性能
        //PolylineData(PolylineData&& other) noexcept = default;
        //PolylineData& operator=(PolylineData&& other) noexcept = default;
        //
        //// 禁用拷贝构造和赋值，避免意外的深拷贝
        //PolylineData(const PolylineData&) = delete;
        //PolylineData& operator=(const PolylineData&) = delete;
    };

    // 实例化线段数据结构
    struct InstanceLineData
    {
        float pos1[3];    // 线段第一个端点位置
        float pos2[3];    // 线段第二个端点位置
        float color[4];   // 线段颜色 (RGBA)
        float width;      // 线段宽度
        float depth;      // 深度值
    };

    // 三角形
    struct GLRENDER_EXPORT TriangleData
    {
        long long id;                       // ID
        std::vector<float> verts;           // x, y, len
        std::vector<unsigned int> indices;  // 索引
        Brush brush;
    };

    // 实例化三角形数据结构
    struct InstanceTriangleData
    {
        float pos1[3];    // 三角形第一个顶点位置
        float pos2[3];    // 三角形第二个顶点位置
        float pos3[3];    // 三角形第三个顶点位置
        float color[4];   // 三角形颜色 (RGBA)
        float depth;      // 深度值
    };

    // 纹理
    struct GLRENDER_EXPORT TextureData
    {
        long long id;                       // ID
        std::vector<float> verts;           //  x, y, u, v
        std::vector<unsigned int> indices;
        unsigned int tex;
        Brush brush;
    };

    // 实例纹理
    struct GLRENDER_EXPORT InstanceTexData
    {
        long long id;                       // ID
        float x, y;          // 位置
        float width, height; // 尺寸
        int   textureLayer;  // 纹理数组层索引
        float alpha;         // 透明度
    };

    // 回调类型定义
    using SnapCb = std::function<void(const RenderSnap&)>;
    using GetMousePtCb = std::function<void(float&, float&)>;
}

#endif // RENDER_COMMON_H