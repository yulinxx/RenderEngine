#ifndef ORTHOGRAPHIC_CAMERA_H
#define ORTHOGRAPHIC_CAMERA_H

#include "GLRenderExport.h"
#include <QSize>
#include <QPointF>

namespace GLRhi
{
    class GLRENDER_EXPORT OrthographicCamera
    {
    public:
        OrthographicCamera();
        ~OrthographicCamera() = default;

        // 设置正交投影参数
        void setOrthographic(float left, float right, float bottom, float top, float nearPlane = -1.0f, float farPlane = 1.0f);
        
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

        // 获取MVP矩阵（Model * View * Projection）
        const float* getModelViewProjectionMatrix() const;
        
        // 获取View矩阵
        const float* getViewMatrix() const;
        
        // 获取Projection矩阵
        const float* getProjectionMatrix() const;

        // 设置是否允许操作
        void setEnableTrans(bool enable);
        bool isEnableTrans() const;

        // 设置模型矩阵（可选，默认为单位矩阵）
        void setModelMatrix(const float* modelMatrix);

    private:
        bool m_bEnableTrans = true;
        
        // 投影矩阵参数
        float m_left = -1.0f;
        float m_right = 1.0f;
        float m_bottom = -1.0f;
        float m_top = 1.0f;
        float m_nearPlane = -1.0f;
        float m_farPlane = 1.0f;
        
        // 视图参数
        float m_dScale = 1.0f;
        float m_dTransX = 0.0f;
        float m_dTransY = 0.0f;
        
        // 矩阵存储
        float m_projectionMat[16] = { 0.0f };
        float m_viewMat[16] = { 0.0f };
        float m_modelMat[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 
                                0.0f, 1.0f, 0.0f, 0.0f, 
                                0.0f, 0.0f, 1.0f, 0.0f, 
                                0.0f, 0.0f, 0.0f, 1.0f };
        float m_mvpMat[16] = { 0.0f };
        
        // 计算正交投影矩阵
        void calculateProjectionMatrix();
        
        // 计算视图矩阵
        void calculateViewMatrix();
        
        // 计算MVP矩阵
        void calculateMVP();
        
        // 矩阵乘法
        void multiplyMatrices(const float* a, const float* b, float* result) const;
    };
}

#endif // ORTHOGRAPHIC_CAMERA_H