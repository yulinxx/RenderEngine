#ifndef MarchCamera_H
#define MarchCamera_H

#include <QPointF>
#include <QSize>
namespace GLRhi
{


class MarchCamera
{
public:
    MarchCamera() = default;

    // 更新相机矩阵（需传入窗口尺寸获取宽高比）
    void updateMatrix(const QSize& viewSize);

    // 屏幕坐标转世界坐标
    QPointF screenToWorld(const QPointF& screenPos, const QSize& viewSize) const;

    // 缩放到指定范围
    void zoomToRange(float minX, float minY, float maxX, float maxY, const QSize& viewSize);

    // 处理平移（窗口坐标差）
    void translate(const QPointF& delta, const QSize& viewSize);

    // 处理缩放（鼠标位置+缩放系数）
    void scale(const QPointF& screenPos, float scaleFactor, const QSize& viewSize);

    // 获取相机矩阵
    const float* getMatrix() const
    {
        return m_cameraMat;
    }

    // 设置是否允许操作
    void setEnableTrans(bool enable)
    {
        m_bEnableTrans = enable;
    }
    bool isEnableTrans() const
    {
        return m_bEnableTrans;
    }

private:
    bool m_bEnableTrans = true;
    float m_cameraMat[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    float m_dScale = 1.0f;
    float m_dTransX = 0.0f;
    float m_dTransY = 0.0f;
};

}
#endif // MarchCamera_H