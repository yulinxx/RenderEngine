#include "Widget/RenderWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QImage>
namespace GLRhi
{
    RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent)
    {
        QSurfaceFormat format;
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setSamples(4); // 启用抗锯齿
        setFormat(format);
        setMouseTracking(true); // 启用鼠标跟踪（无需按下也能触发move事件）
    }

    RenderWidget::~RenderWidget()
    {
        makeCurrent();
        m_renderManager.cleanup(); // 清理渲染资源
        doneCurrent();
    }

    void RenderWidget::initializeGL()
    {
        if (!initializeOpenGLFunctions())
        {
            qFatal("Initialize OpenGL functions failed!");
            return;
        }

        // 初始化OpenGL状态
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFFFFFF);

        // 初始化抗锯齿状态
        if (m_bAntiAliasEnabled)
        {
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }

        // 初始化线框模式状态
        glPolygonMode(GL_FRONT_AND_BACK, m_bWireframeMode ? GL_LINE : GL_FILL);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色背景

        // 初始化渲染管理器
        if (!m_renderManager.initialize(this))
        {
            qFatal("Initialize render manager failed!");
        }

        // 初始化相机矩阵
        m_camera.updateMatrix(size());
        checkGLError("initializeGL");
    }

    void RenderWidget::resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
        m_camera.updateMatrix(QSize(w, h));
        checkGLError("resizeGL");
    }

    void RenderWidget::paintGL()
    {
        const Brush& b = m_renderManager.getBackgroundColor();
        glClearColor(b.r(), b.g(), b.b(), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_renderManager.render(m_camera.getMatrix());

        checkGLError("paintGL");
    }

    void RenderWidget::keyPressEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
        case Qt::Key_F1:
            // F1功能：启用/关闭抗锯齿
            makeCurrent();
            m_bAntiAliasEnabled = !m_bAntiAliasEnabled;
            if (m_bAntiAliasEnabled)
            {
                glEnable(GL_MULTISAMPLE);
            }
            else
            {
                glDisable(GL_MULTISAMPLE);
            }
            update();
            break;
        case Qt::Key_F2:
            // F2功能：启用/关闭线框模式
            makeCurrent();
            m_bWireframeMode = !m_bWireframeMode;
            if (m_bWireframeMode)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            update();
            break;
        case Qt::Key_F3:
            // F3功能：留空实现
            break;
        case Qt::Key_F4:
            // F4功能：留空实现
            break;
        case Qt::Key_F5:
            makeCurrent();
            m_renderManager.genFakeData();
            update();
            break;
        default:
            break;
        }

        QOpenGLWidget::keyPressEvent(event);
    }

    void RenderWidget::keyReleaseEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Control)
        {
        }
        QOpenGLWidget::keyReleaseEvent(event);
    }

    void RenderWidget::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::MiddleButton && m_camera.isEnableTrans())
        {
            m_lastMousePos = event->pos();
            m_bDragging = true;
            setCursor(Qt::ClosedHandCursor); // 拖动时显示"关闭的手"光标
        }
        QOpenGLWidget::mousePressEvent(event);
    }

    void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::MiddleButton)
        {
            m_bDragging = false;
            setCursor(Qt::ArrowCursor); // 释放时恢复默认光标
        }
        QOpenGLWidget::mouseReleaseEvent(event);
    }

    void RenderWidget::mouseMoveEvent(QMouseEvent* event)
    {
        // 触发鼠标位置回调（转换为世界坐标）
        if (m_mouseCb)
        {
            QPointF worldPos = m_camera.screenToWorld(event->pos(), size());
            float x = worldPos.x();
            float y = worldPos.y();
            m_mouseCb(x, y);
        }

        // 处理拖动平移
        if (m_bDragging && m_camera.isEnableTrans())
        {
            QPointF delta = event->pos() - m_lastMousePos;
            m_camera.translate(delta, size()); // 相机平移
            m_lastMousePos = event->pos();
            update(); // 触发重绘
        }
        QOpenGLWidget::mouseMoveEvent(event);
    }

    void RenderWidget::wheelEvent(QWheelEvent* event)
    {
        if (!m_camera.isEnableTrans())
        {
            QOpenGLWidget::wheelEvent(event);
            return;
        }

        // 计算缩放因子（上滚放大，下滚缩小）
        float scaleFactor = event->angleDelta().y() > 0 ? 1.1f : 0.9f;
        m_camera.scale(event->position(), scaleFactor, size()); // 相机缩放
        update(); // 触发重绘
        QOpenGLWidget::wheelEvent(event);
    }

    void RenderWidget::setBackgroundColor(const float r, const float g, const float b)
    {
        m_renderManager.setBackgroundColor(Brush(r, g, b));
        update();
    }

    //void RenderWidget::setLineDepth(float depth)
    //{
    //    m_renderManager.getLineRenderer()->setDepth(depth);
    //}
    //
    //void RenderWidget::setLineBDepth(float depth)
    //{
    //    m_renderManager.getLineBRenderer()->setDepth(depth);
    //}
    //
    //void RenderWidget::setFillDepth(float depth)
    //{
    //    m_renderManager.getFillRenderer()->setDepth(depth);
    //}
    //
    //void RenderWidget::setImgDepth(float depth)
    //{
    //    m_renderManager.getImageRenderer()->setDepth(depth);
    //}

    void RenderWidget::updateLineDataBuffer(float* data, size_t count, Brush color)
    {
        makeCurrent();
        //m_renderManager.getLineRenderer()->updateData(data, count, color);
        doneCurrent();
        update();
    }

    void RenderWidget::updateLineBDataBuffer(float* data, size_t count, Brush color,
        int lineType, float dashScale, float thickness)
    {
        makeCurrent();
        //m_renderManager.getLineBRenderer()->updateData(data, count, color, lineType, dashScale, thickness);
        doneCurrent();
        update();
    }

    void RenderWidget::updateFillDataBuffer(float* data, size_t count, Brush color)
    {
        makeCurrent();
        //m_renderManager.getFillRenderer()->updateData(data, count, color);
        doneCurrent();
        update();
    }

    void RenderWidget::updateImageTexture(const float* vertices, size_t vertexCount,
        const unsigned char* imgData, int width, int height)
    {
        makeCurrent();
        //m_renderManager.getImageRenderer()->updateData(vertices, vertexCount, imgData, width, height);
        doneCurrent();
        update();
    }

    void RenderWidget::setEnableTrans(bool b)
    {
        m_camera.setEnableTrans(b);
        setCursor(b ? Qt::ArrowCursor : Qt::ForbiddenCursor); // 禁用时显示禁止光标
    }

    void RenderWidget::setShowCheckerboard(bool b)
    {
        //m_renderManager.getCheckerboardRenderer().setVisible(b);
        update();
    }

    void RenderWidget::setCheckerboardSz(int n)
    {
        //m_renderManager.getCheckerboardRenderer().setSize(static_cast<float>(n));
        update();
    }

    void RenderWidget::setShowCheckerboardColor(Brush colorA, Brush colorB)
    {
        //m_renderManager.getCheckerboardRenderer().setColors(colorA, colorB);
        update();
    }

    void RenderWidget::zoomToRange(float minX, float minY, float maxX, float maxY)
    {
        m_camera.zoomToRange(minX, minY, maxX, maxY, size());
        update();
    }

    void RenderWidget::clear()
    {
        makeCurrent();
        m_renderManager.cleanup(); // 清理渲染资源
        m_renderManager.initialize(this); // 重新初始化渲染器
        m_camera = MarchCamera(); // 重置相机
        m_camera.updateMatrix(size());
        doneCurrent();
        update();
    }

    void RenderWidget::grabSnap(const SnapCb& cb, const QSize& pixelSize)
    {
        makeCurrent();
        QSize sz = pixelSize.isEmpty() ? size() : pixelSize;
        if (sz.isEmpty())
        {
            doneCurrent();
            return;
        }

        // 创建FBO用于离屏渲染
        GLuint fbo, colorTex, depthRb;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // 创建颜色纹理
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sz.width(), sz.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        // 创建深度缓冲区
        glGenRenderbuffers(1, &depthRb);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sz.width(), sz.height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb);

        // 检查FBO完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            qWarning() << "FBO创建失败";
            glDeleteRenderbuffers(1, &depthRb);
            glDeleteTextures(1, &colorTex);
            glDeleteFramebuffers(1, &fbo);
            doneCurrent();
            return;
        }

        // 离屏渲染
        glViewport(0, 0, sz.width(), sz.height());
        const Brush& b = m_renderManager.getBackgroundColor();
        glClearColor(b.r(), b.g(), b.b(), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_renderManager.render(m_camera.getMatrix());

        // 读取像素数据
        QImage img(sz, QImage::Format_ARGB32);
        glReadPixels(0, 0, sz.width(), sz.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
        img = img.mirrored(false, true); // 垂直翻转（OpenGL原点在左下角，QImage在左上角）

        // 计算截图对应的世界坐标范围
        QPointF worldTopLeft = m_camera.screenToWorld(QPointF(0, 0), sz);
        QPointF worldBottomRight = m_camera.screenToWorld(QPointF(sz.width(), sz.height()), sz);
        QRectF worldRect(worldTopLeft, worldBottomRight);

        // 触发回调
        if (cb)
        {
            cb({ img, worldRect });
        }

        // 清理FBO资源
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteRenderbuffers(1, &depthRb);
        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);

        doneCurrent();
    }

    void RenderWidget::setMousePosCb(const GetMousePtCb& cb)
    {
        m_mouseCb = cb;
    }

    void RenderWidget::checkGLError(const QString& context)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            QString errorStr;
            switch (err)
            {
            case GL_INVALID_ENUM: errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            default: errorStr = QString("Unknow Error 0x%1").arg(err, 0, 16);
            }
            qWarning() << "OpenGL Error[" << context << "]:" << errorStr;
        }
    }
}