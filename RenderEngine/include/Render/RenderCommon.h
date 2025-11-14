#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

#include "GLRenderExport.h"
#include <QImage>
#include <QRectF>
#include <QPointF>
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
    struct GLRENDER_EXPORT PolylineData
    {
        std::vector<float> verts{};          //  x, y, len
        std::vector<size_t> count;          // 每条线段顶点数
        Brush brush{ 0.0, 0.0, 0.0, 1.0, 0.0 }; // 渲染信息
    };

    // 三角形
    struct GLRENDER_EXPORT TriangleData
    {
        std::vector<float> verts;           // x, y, len
        std::vector<unsigned int> indices;  // 索引
        Brush brush;
    };

    // 纹理
    struct GLRENDER_EXPORT TextureData
    {
        std::vector<float> verts;        //  x, y, u, v
        std::vector<unsigned int> indices;
        unsigned int tex;
        Brush brush;
    };

    // 实例纹理
    struct GLRENDER_EXPORT InstanceTexData
    {
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