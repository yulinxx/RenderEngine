#include "Common/OrthographicCamera.h"

namespace GLRhi
{
    OrthographicCamera::OrthographicCamera()
    {
        // 初始化矩阵
        calculateProjectionMatrix();
        calculateViewMatrix();
        calculateMVP();
    }

    void OrthographicCamera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        m_left = left;
        m_right = right;
        m_bottom = bottom;
        m_top = top;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        
        calculateProjectionMatrix();
        calculateMVP();
    }

    void OrthographicCamera::updateMatrix(const QSize& viewSize)
    {
        if (viewSize.isEmpty())
            return;

        // 根据窗口尺寸调整投影矩阵
        float aspectRatio = static_cast<float>(viewSize.width()) / static_cast<float>(viewSize.height());
        
        // 保持原有的缩放和平移效果
        calculateViewMatrix();
        calculateMVP();
    }

    QPointF OrthographicCamera::screenToWorld(const QPointF& screenPos, const QSize& viewSize) const
    {
        if (viewSize.isEmpty())
            return QPointF(0, 0);

        // 屏幕坐标转换到NDC坐标 [-1, 1]
        float ndcX = (2.0f * screenPos.x() / viewSize.width()) - 1.0f;
        float ndcY = 1.0f - (2.0f * screenPos.y() / viewSize.height()); // 注意Y轴翻转

        // 应用逆变换：NDC -> 世界坐标
        float worldX = ndcX / (m_projectionMat[0] * m_dScale) - m_dTransX;
        float worldY = ndcY / (m_projectionMat[5] * m_dScale) - m_dTransY;

        return QPointF(worldX, worldY);
    }

    void OrthographicCamera::zoomToRange(float minX, float minY, float maxX, float maxY, const QSize& viewSize)
    {
        if (viewSize.isEmpty())
            return;

        float worldWidth = maxX - minX;
        float worldHeight = maxY - minY;
        float aspectRatio = static_cast<float>(viewSize.width()) / static_cast<float>(viewSize.height());

        // 计算合适的缩放比例，确保整个范围可见
        float scaleX = 2.0f / worldWidth;
        float scaleY = 2.0f / worldHeight;
        
        // 根据宽高比调整，防止拉伸
        if (worldWidth / worldHeight > aspectRatio)
        {
            // 宽度是限制因素
            m_dScale = scaleX;
        }
        else
        {
            // 高度是限制因素
            m_dScale = scaleY;
        }

        // 居中显示
        m_dTransX = -(minX + worldWidth / 2.0f);
        m_dTransY = -(minY + worldHeight / 2.0f);

        updateMatrix(viewSize);
    }

    void OrthographicCamera::translate(const QPointF& delta, const QSize& viewSize)
    {
        if (!m_bEnableTrans || viewSize.isEmpty())
            return;

        // 计算世界坐标中的平移量
        float worldDeltaX = delta.x() / (m_dScale * viewSize.width() / 2.0f);
        float worldDeltaY = -delta.y() / (m_dScale * viewSize.height() / 2.0f); // Y轴翻转

        m_dTransX += worldDeltaX;
        m_dTransY += worldDeltaY;

        updateMatrix(viewSize);
    }

    void OrthographicCamera::scale(const QPointF& screenPos, float scaleFactor, const QSize& viewSize)
    {
        if (!m_bEnableTrans || viewSize.isEmpty())
            return;

        // 获取缩放前的鼠标世界坐标
        QPointF worldPos = screenToWorld(screenPos, viewSize);

        // 应用缩放
        m_dScale *= scaleFactor;
        
        // 调整平移，使缩放围绕鼠标位置进行
        QPointF newWorldPos = screenToWorld(screenPos, viewSize);
        m_dTransX += worldPos.x() - newWorldPos.x();
        m_dTransY += worldPos.y() - newWorldPos.y();

        updateMatrix(viewSize);
    }

    const float* OrthographicCamera::getModelViewProjectionMatrix() const
    {
        return m_mvpMat;
    }

    const float* OrthographicCamera::getViewMatrix() const
    {
        return m_viewMat;
    }

    const float* OrthographicCamera::getProjectionMatrix() const
    {
        return m_projectionMat;
    }

    void OrthographicCamera::setEnableTrans(bool enable)
    {
        m_bEnableTrans = enable;
    }

    bool OrthographicCamera::isEnableTrans() const
    {
        return m_bEnableTrans;
    }

    void OrthographicCamera::setModelMatrix(const float* modelMatrix)
    {
        if (modelMatrix)
        {
            for (int i = 0; i < 16; ++i)
            {
                m_modelMat[i] = modelMatrix[i];
            }
            calculateMVP();
        }
    }

    void OrthographicCamera::calculateProjectionMatrix()
    {
        // 正交投影矩阵
        float tx = -(m_right + m_left) / (m_right - m_left);
        float ty = -(m_top + m_bottom) / (m_top - m_bottom);
        float tz = -(m_farPlane + m_nearPlane) / (m_farPlane - m_nearPlane);

        m_projectionMat[0] = 2.0f / (m_right - m_left);
        m_projectionMat[1] = 0.0f;
        m_projectionMat[2] = 0.0f;
        m_projectionMat[3] = tx;

        m_projectionMat[4] = 0.0f;
        m_projectionMat[5] = 2.0f / (m_top - m_bottom);
        m_projectionMat[6] = 0.0f;
        m_projectionMat[7] = ty;

        m_projectionMat[8] = 0.0f;
        m_projectionMat[9] = 0.0f;
        m_projectionMat[10] = -2.0f / (m_farPlane - m_nearPlane);
        m_projectionMat[11] = tz;

        m_projectionMat[12] = 0.0f;
        m_projectionMat[13] = 0.0f;
        m_projectionMat[14] = 0.0f;
        m_projectionMat[15] = 1.0f;
    }

    void OrthographicCamera::calculateViewMatrix()
    {
        // 视图矩阵：缩放和平移
        m_viewMat[0] = m_dScale;
        m_viewMat[1] = 0.0f;
        m_viewMat[2] = 0.0f;
        m_viewMat[3] = m_dScale * m_dTransX;

        m_viewMat[4] = 0.0f;
        m_viewMat[5] = m_dScale;
        m_viewMat[6] = 0.0f;
        m_viewMat[7] = m_dScale * m_dTransY;

        m_viewMat[8] = 0.0f;
        m_viewMat[9] = 0.0f;
        m_viewMat[10] = 1.0f;
        m_viewMat[11] = 0.0f;

        m_viewMat[12] = 0.0f;
        m_viewMat[13] = 0.0f;
        m_viewMat[14] = 0.0f;
        m_viewMat[15] = 1.0f;
    }

    void OrthographicCamera::calculateMVP()
    {
        // MVP = Projection * View * Model
        // 先计算 View * Model
        float viewModel[16];
        multiplyMatrices(m_viewMat, m_modelMat, viewModel);
        // 再计算 Projection * (View * Model)
        multiplyMatrices(m_projectionMat, viewModel, m_mvpMat);
    }

    void OrthographicCamera::multiplyMatrices(const float* a, const float* b, float* result) const
    {
        // 4x4矩阵乘法
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
                }
            }
        }
    }
}