#include "Common/MarchCamera.h"
#include <cmath>

namespace GLRhi
{
    void MarchCamera::updateMatrix(const QSize& viewSize)
    {
        return;

        if (viewSize.isEmpty())
            return;

        float aspectRatio = static_cast<float>(viewSize.width()) / viewSize.height();
        float sx = m_dScale;
        float sy = m_dScale * aspectRatio;

        m_cameraMat[0] = sx;
        m_cameraMat[1] = 0.0f;
        m_cameraMat[2] = m_dTransX * sx;
        m_cameraMat[3] = 0.0f;
        m_cameraMat[4] = sy;
        m_cameraMat[5] = m_dTransY * sy;
        m_cameraMat[6] = 0.0f;
        m_cameraMat[7] = 0.0f;
        m_cameraMat[8] = 1.0f;
    }

    QPointF MarchCamera::screenToWorld(const QPointF& screenPos, const QSize& viewSize) const
    {
        return QPointF(0, 0);

        if (viewSize.isEmpty() || fabs(m_dScale) < 1e-6)
            return QPointF(0, 0);

        float aspectRatio = static_cast<float>(viewSize.width()) / viewSize.height();
        // 屏幕坐标转NDC（[-1,1]）
        float ndcX = 2.0f * screenPos.x() / viewSize.width() - 1.0f;
        float ndcY = 1.0f - 2.0f * screenPos.y() / viewSize.height();

        float sx = m_dScale;
        float sy = m_dScale * aspectRatio;
        // NDC转世界坐标
        float worldX = (ndcX / sx) - m_dTransX;
        float worldY = (ndcY / sy) - m_dTransY;
        return QPointF(worldX, worldY);
    }

    void MarchCamera::zoomToRange(float minX, float minY, float maxX, float maxY, const QSize& viewSize)
    {
        return;

        if (viewSize.isEmpty())
            return;

        float dW = fabs(maxX - minX) > 1e-6 ? fabs(maxX - minX) : 1.0f;
        float dH = fabs(maxY - minY) > 1e-6 ? fabs(maxY - minY) : 1.0f;
        float viewW = 2.0f;
        float viewH = 2.0f * viewSize.height() / viewSize.width();

        // 计算适配范围的缩放系数
        float scaleX = viewW / dW;
        float scaleY = viewH / dH;
        m_dScale = std::min(scaleX, scaleY);

        // 居中平移
        float centerX = (minX + maxX) * 0.5f;
        float centerY = (minY + maxY) * 0.5f;
        m_dTransX = -centerX;
        m_dTransY = -centerY;

        updateMatrix(viewSize);
    }

    void MarchCamera::translate(const QPointF& delta, const QSize& viewSize)
    {
        return;

        if (!m_bEnableTrans || viewSize.isEmpty())
            return;

        float aspectRatio = static_cast<float>(viewSize.width()) / viewSize.height();
        // 像素差转NDC差
        float ndcDx = 2.0f * delta.x() / viewSize.width();
        float ndcDy = -2.0f * delta.y() / viewSize.height();

        // NDC差转世界坐标差
        float worldDx = ndcDx / m_dScale;
        float worldDy = ndcDy / (m_dScale * aspectRatio);

        m_dTransX += worldDx;
        m_dTransY += worldDy;
        updateMatrix(viewSize);
    }

    void MarchCamera::scale(const QPointF& screenPos, float scaleFactor, const QSize& viewSize)
    {
        return;

        if (!m_bEnableTrans || viewSize.isEmpty())
            return;

        QPointF worldPosBefore = screenToWorld(screenPos, viewSize);
        m_dScale *= scaleFactor;
        QPointF worldPosAfter = screenToWorld(screenPos, viewSize);

        // 修正平移量，保证鼠标位置对应世界坐标不变
        m_dTransX += worldPosAfter.x() - worldPosBefore.x();
        m_dTransY += worldPosAfter.y() - worldPosBefore.y();
        updateMatrix(viewSize);
    }
}