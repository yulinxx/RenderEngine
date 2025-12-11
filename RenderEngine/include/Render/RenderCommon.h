#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

#include <QImage>
#include <functional>
#include <vector>
#include "Common/DllSet.h"
#include "Common/Brush.h"

namespace GLRhi
{
    // 截图数据结构体
    struct GLRENDER_API RenderSnap
    {
        QImage image;
        QRectF worldRect;
    };

    // 多段线
    struct GLRENDER_API PolylineData
    {
        std::vector<long long> vId;             // ID
        std::vector<size_t> vCount;             // 线段顶点数
        std::vector<float> vVerts{};            //  x, y, len
        Brush brush{ 0.0, 0.0, 0.0, 1.0, 0.0 }; // 渲染信息
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
    struct GLRENDER_API TriangleData
    {
        long long id;                       // ID
        std::vector<float> vVerts;          // x, y, len
        std::vector<unsigned int> vIndices; // 索引
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
    struct GLRENDER_API TextureData
    {
        long long id;                       // ID
        std::vector<float> vVerts;          // x, y, u, v
        std::vector<unsigned int> vIndices;
        unsigned int tex;
        Brush brush;
    };

    // 实例纹理
    struct GLRENDER_API InstanceTexData
    {
        long long id;        // ID
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