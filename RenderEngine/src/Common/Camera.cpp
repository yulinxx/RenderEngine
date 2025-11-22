#include "Common/Camera.h"
#include <cmath>
#include <algorithm>

namespace GLRhi
{
    // -----------------------------------------------------------------
    static void copy3x3To4x4(const float src[9], float dst[16])
    {
        // 列优先： 0 3 6
        //         1 4 7
        //         2 5 8
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = 0;
        dst[4] = src[3]; dst[5] = src[4]; dst[6] = src[5]; dst[7] = 0;
        dst[8] = src[6]; dst[9] = src[7]; dst[10] = src[8]; dst[11] = 0;
        dst[12] = 0;      dst[13] = 0;      dst[14] = 0;      dst[15] = 1;
    }

    // -----------------------------------------------------------------
    void Camera::updateMatrix(const QSize& viewSz)
    {
        if (viewSz.isEmpty())
            return;

        const float w = static_cast<float>(viewSz.width());
        const float h = static_cast<float>(viewSz.height());
        const float aspect = w / h;

        // 正交投影：把 [-1,1]×[-1,1]（NDC）映射到世界坐标的 [-1/scale, 1/scale]×...
        const float sx = m_dScale;                 // X 方向缩放
        const float sy = m_dScale * aspect;        // Y 方向缩放（保持比例）

        float ortho[16] = {
            sx, 0, 0, 0,
            0, sy, 0, 0,
            0,  0, 1, 0,
            0,  0, 0, 1
        };

        // 平移矩阵（列优先）
        // float trans[16] = {
        //     1, 0, 0, 0,
        //     0, 1, 0, 0,
        //     0, 0, 1, 0,
        //     m_dTransX * sx, m_dTransY * sy, 0, 1
        // };

        float trans[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            m_dTransX, m_dTransY, 0, 1 };

        // 矩阵乘法：ortho * trans  (列优先 → 先写左乘)
        float tmp[16]{};
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
            {
                tmp[i * 4 + j] =
                    ortho[i * 4 + 0] * trans[0 * 4 + j] +
                    ortho[i * 4 + 1] * trans[1 * 4 + j] +
                    ortho[i * 4 + 2] * trans[2 * 4 + j] +
                    ortho[i * 4 + 3] * trans[3 * 4 + j];
            }

        std::copy(tmp, tmp + 16, m_cameraMat);
    }

    // -----------------------------------------------------------------
    QPointF Camera::screenToWorld(const QPointF& screenPos,
        const QSize& viewSz) const
    {
        if (viewSz.isEmpty() || std::fabs(m_dScale) < 1e-6f)
            return QPointF(0, 0);

        const float w = static_cast<float>(viewSz.width());
        const float h = static_cast<float>(viewSz.height());

        // 1. 屏幕 → NDC [-1,1]
        float ndcX = 2.0f * screenPos.x() / w - 1.0f;
        float ndcY = -2.0f * screenPos.y() / h + 1.0f;   // Y 向下

        // 2. 构造逆矩阵（正交+平移 → 逆非常简单）
        //    M = [ sx  0  0  tx*sx ]
        //        [ 0  sy  0  ty*sy ]
        //        [ 0   0  1   0   ]
        //        [ 0   0  0   1   ]
        //    M⁻¹ = [ 1/sx   0    0   -tx ]
        //          [  0   1/sy   0   -ty ]
        //          [  0    0    1    0  ]
        //          [  0    0    0    1  ]
        const float aspect = w / h;
        const float sx = m_dScale;
        const float sy = m_dScale * aspect;

        const float invSX = 1.0f / sx;
        const float invSY = 1.0f / sy;

        float worldX = ndcX * invSX + m_dTransX;
        float worldY = ndcY * invSY + m_dTransY;

        return QPointF(worldX, worldY);
    }

    // -----------------------------------------------------------------
    void Camera::zoomToRange(float minX, float minY,
        float maxX, float maxY, const QSize& viewSz)
    {
        if (viewSz.isEmpty()) return;

        const float dW = std::max(std::fabs(maxX - minX), 1e-6f);
        const float dH = std::max(std::fabs(maxY - minY), 1e-6f);

        // NDC 范围在 X、Y 方向分别是 [-1,1] → 宽度 2
        const float viewW = 2.0f;
        const float viewH = 2.0f * viewSz.height() / viewSz.width();

        const float scaleX = viewW / dW;
        const float scaleY = viewH / dH;
        m_dScale = std::min(scaleX, scaleY);

        const float centerX = (minX + maxX) * 0.5f;
        const float centerY = (minY + maxY) * 0.5f;
        m_dTransX = -centerX;
        m_dTransY = -centerY;

        updateMatrix(viewSz);
    }

    // -----------------------------------------------------------------
    void Camera::translate(const QPointF& delta, const QSize& viewSz)
    {
        if (!m_bEnableTrans || viewSz.isEmpty())
            return;

        const float w = static_cast<float>(viewSz.width());
        const float h = static_cast<float>(viewSz.height());

        // 像素差 → NDC 差
        const float ndcDx = 2.0f * delta.x() / w;
        const float ndcDy = -2.0f * delta.y() / h;   // Y 向下

        // 修改：移除Y方向的aspect除法，使X和Y方向移动一致
        const float worldDx = ndcDx;
        const float worldDy = ndcDy;

        m_dTransX += worldDx;
        m_dTransY += worldDy;

        updateMatrix(viewSz);
    }

    // -----------------------------------------------------------------
    void Camera::scale(const QPointF& screenPos, float scaleFactor,
        const QSize& viewSz)
    {
        if (!m_bEnableTrans || viewSz.isEmpty())
            return;

        // 1. 缩放前的世界坐标
        const QPointF worldBefore = screenToWorld(screenPos, viewSz);

        // 2. 改变缩放
        m_dScale *= scaleFactor;

        // 3. 缩放后的世界坐标（同一屏幕点）
        const QPointF worldAfter = screenToWorld(screenPos, viewSz);

        // 4. 补偿平移，使该点保持不动
        m_dTransX -= (worldAfter.x() - worldBefore.x());
        m_dTransY -= (worldAfter.y() - worldBefore.y());

        updateMatrix(viewSz);
    }

    // -----------------------------------------------------------------
    const float* Camera::getMatrix() const
    {
        return m_cameraMat;   // 列优先 4×4
    }

    // -----------------------------------------------------------------
    void Camera::setEnableTrans(bool enable)
    {
        m_bEnableTrans = enable;
    }

    bool Camera::isEnableTrans() const
    {
        return m_bEnableTrans;
    }
}