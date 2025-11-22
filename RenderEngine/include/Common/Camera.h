#ifndef CAMERA_H
#define CAMERA_H

#include "GLRenderExport.h"
#include <QPointF>
#include <QSize>

namespace GLRhi
{
    class GLRENDER_EXPORT Camera
    {
    public:
        Camera() = default;

        // 更新相机矩阵（需传入窗口尺寸获取宽高比）
        void updateMatrix(const QSize& viewSz);

        // 屏幕坐标 → 世界坐标
        QPointF screenToWorld(const QPointF& screenPos, const QSize& viewSz) const;

        // 缩放到指定范围（自动居中）
        void zoomToRange(float minX, float minY, float maxX, float maxY,
            const QSize& viewSz);

        // 平移（窗口像素差）
        void translate(const QPointF& delta, const QSize& viewSz);

        // 缩放（鼠标位置 + 缩放系数），保持鼠标指向的世界点不动
        void scale(const QPointF& screenPos, float scaleFactor,
            const QSize& viewSz);

        // 获取 4×4 列优先矩阵指针
        const float* getMatrix() const;

        // 是否允许平移/缩放
        void setEnableTrans(bool enable);
        bool isEnableTrans() const;

    private:
        bool  m_bEnableTrans = true;

        // 4×4 列优先正交矩阵
        float m_cameraMat[16] = {
            1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
        };

        // 为了保持原有接口的语义，保留这几个“逻辑”变量
        float m_dScale = 1.0f;   // 统一缩放（X 方向）
        float m_dTransX = 0.0f;
        float m_dTransY = 0.0f;
    };
}
#endif // CAMERA_H
