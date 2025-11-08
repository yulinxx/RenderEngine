#ifndef RENDERCOMMON_H
#define RENDERCOMMON_H

#include <QImage>
#include <QRectF>
#include <QPointF>
#include <functional>
#include <vector>
namespace GLRhi
{


// 截图数据结构体
struct RenderSnap 
{
    QImage image;
    QRectF worldRect;
};

// 回调类型定义
using SnapCb = std::function<void(const RenderSnap&)>;
using GetMousePtCb = std::function<void(float&, float&)>;

}
#endif // RENDERCOMMON_H