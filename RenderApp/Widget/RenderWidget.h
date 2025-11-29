#ifndef RENDER_WIDGET_H
#define RENDER_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

#include "Common/Camera.h"
#include "Common/Brush.h"
#include "Render/RenderManager.h"
#include "Render/RenderCommon.h"

namespace GLRhi
{
    class RenderWidget final : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
    {
        Q_OBJECT
    public:
        explicit RenderWidget(QWidget* parent = nullptr);
        ~RenderWidget() override;

        // 背景颜色设置
        void setBackgroundColor(const float r, const float g, const float b);

        // 抗锯齿控制
        void setAntiAliasEnabled(bool enabled);
        bool isAntiAliasEnabled() const;

        // 线段数据更新
        void updateLineDataBuffer(float* data, size_t count, Brush color = Brush{});
        void updateLineBDataBuffer(float* data, size_t count, Brush color,
            int lineType = 100, float dashScale = 1.0f, float thickness = 0.0015f);

        // 填充数据更新
        void updateFillDataBuffer(float* data, size_t count, Brush color = Brush{});

        // 图像数据更新
        void updateImageTexture(const float* vertices, size_t vertexCount,
            const unsigned char* imgData, int width, int height);

        // 交互控制
        void setEnableTrans(bool b = true);

        // 棋盘格控制
        void setShowCheckerboard(bool b);
        void setCheckerboardSz(int n);
        void setShowCheckerboardColor(Brush brushA, Brush brushB);

        // 视图控制
        void zoomToRange(float minX, float minY, float maxX, float maxY);

        // 清理所有渲染数据
        void clear();

        // 截图功能
        void grabSnap(const SnapCb& cb, const QSize& pixelSize = QSize(0, 0));

        // 鼠标位置回调设置
        void setMousePosCb(const GetMousePtCb& cb);

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

    public:
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

    private:
        void checkGLError(const QString& context = "");

    private:
        Camera m_camera;                    // 相机控制器
        RenderManager m_renderManager;      // 渲染管理器
        GetMousePtCb m_mouseCb = nullptr;   // 鼠标位置回调
        QPointF m_posLast;                  // 鼠标按下位置
        bool m_bDragging = false;           // 拖动状态标记
        bool m_bAntiAlias = true;           // 抗锯齿启用状态
        bool m_bWireframeMode = false;      // 线框模式状态
    };
}
#endif // RENDER_WIDGET_H