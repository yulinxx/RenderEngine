#include "Widget/RenderWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QImage>

#include <cstdlib>
#include <ctime>

namespace GLRhi
{
    RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent)
    {
        // 初始化随机数种子
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        QSurfaceFormat format;
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setSamples(4); // 启用抗锯齿
        setFormat(format);
        setMouseTracking(true);
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
        //glEnable(GL_PRIMITIVE_RESTART);
        //glPrimitiveRestartIndex(0xFFFFFFFF);

        // 初始化抗锯齿状态
        m_bAntiAlias ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);

        glPolygonMode(GL_FRONT_AND_BACK, m_bWireframeMode ? GL_LINE : GL_FILL);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // 初始化渲染管理器
        if (!m_renderManager.initialize(this))
            qFatal("Initialize render manager failed!");

        // 初始化伪数据生成器
        m_dataGen = std::make_unique<FakeDataProvider>();
        m_dataGen->initialize();
        
        m_instanceLineFakeData = std::make_unique<InstanceLineFakeData>();
        m_instanceTriangleFakeData = std::make_unique<InstanceTriangleFakeData>();
        
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

        //checkGLError("paintGL");
    }

    void RenderWidget::keyPressEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
        case Qt::Key_F1:
        {
            // F1：启用/关闭抗锯齿
            setAntiAliasEnabled(!m_bAntiAlias);
            qDebug() << "AntiAliasEnabled:" << m_bAntiAlias;
        }
        break;
        case Qt::Key_F2:
        {
            // F2：启用/关闭线框模式
            makeCurrent();
            m_bWireframeMode = !m_bWireframeMode;

            glPolygonMode(GL_FRONT_AND_BACK, m_bWireframeMode ? GL_LINE : GL_FILL);

            update();
            qDebug() << "WireframeMode:" << m_bWireframeMode;
        }
        break;
        case Qt::Key_F3:
        {
            // F3：使用随机数生成棋盘格颜色和大小
            // 生成随机大小（范围：8-32）
            int nRandomSz = 8 + std::rand() % 25; // 8到32之间的随机数
            setCheckerboardSz(nRandomSz);

            // 生成两个随机颜色
            float r1 = static_cast<float>(std::rand()) / RAND_MAX;
            float g1 = static_cast<float>(std::rand()) / RAND_MAX;
            float b1 = static_cast<float>(std::rand()) / RAND_MAX;
            float a1 = 1.0f; // 不透明

            float r2 = static_cast<float>(std::rand()) / RAND_MAX;
            float g2 = static_cast<float>(std::rand()) / RAND_MAX;
            float b2 = static_cast<float>(std::rand()) / RAND_MAX;
            float a2 = 1.0f; // 不透明

            // 确保两个颜色有足够的对比度
            float brightnessDiff = std::abs((r1 + g1 + b1) - (r2 + g2 + b2));
            if (brightnessDiff < 0.5f)
            {
                // 如果对比度不够，调整第二个颜色使其更亮或更暗
                float avgBrightness1 = (r1 + g1 + b1) / 3.0f;
                if (avgBrightness1 > 0.5f)
                {
                    // 第一个颜色较亮，第二个颜色变暗
                    r2 *= 0.3f;
                    g2 *= 0.3f;
                    b2 *= 0.3f;
                }
                else
                {
                    // 第一个颜色较暗，第二个颜色变亮
                    r2 = 1.0f - r2 * 0.3f;
                    g2 = 1.0f - g2 * 0.3f;
                    b2 = 1.0f - b2 * 0.3f;
                }
            }

            Brush brushA(r1, g1, b1, a1);
            Brush brushB(r2, g2, b2, a2);
            setShowCheckerboardColor(brushA, brushB);
            qDebug() << "CheckerboardSz:" << nRandomSz;
        }
        break;
        case Qt::Key_F4:
            // F4：随机动态数据
            m_renderManager.dataCRUD();
            qDebug() << "RandomDynamicData";
            break;
        case Qt::Key_F5:
        {
            makeCurrent();
            // 检查修饰键组合
            bool hasCtrl = event->modifiers() & Qt::ControlModifier;
            bool hasAlt = event->modifiers() & Qt::AltModifier;
            bool hasShift = event->modifiers() & Qt::ShiftModifier;

            // 根据不同的修饰键组合执行不同的操作
            if (hasCtrl && hasAlt && hasShift)
            {
                // Ctrl+Alt+Shift+F5: 生成更多的伪数据
                //m_renderManager.genFakeData(2000); // 假设genFakeData可以接受参数表示数据量
                qDebug() << "GenFakeData (Ctrl+Alt+Shift+F5) - More data";
            }
            else if (hasCtrl && hasAlt)
            {
                // Ctrl+Alt+F5: 生成中等数量的伪数据
                //m_renderManager.genFakeData(1000);
                qDebug() << "GenFakeData (Ctrl+Alt+F5) - Medium data";
            }
            else if (hasCtrl && hasShift)
            {
                // Ctrl+Shift+F5: 生成特殊类型的伪数据
                //m_renderManager.genSpecialFakeData(); // 假设存在此方法
                qDebug() << "GenSpecialFakeData (Ctrl+Shift+F5)";
            }
            else if (hasAlt && hasShift)
            {
                // Alt+Shift+F5: 清空现有数据并生成新数据
                //m_renderManager.clearAllData(); // 假设存在此方法
                //m_renderManager.genFakeData();
                qDebug() << "Clear and GenFakeData (Alt+Shift+F5)";
            }
            else if (hasCtrl)
            {
                // Ctrl+F5: 生成少量伪数据
                //m_renderManager.genFakeData(500);
                qDebug() << "GenFakeData (Ctrl+F5) - Less data";
            }
            else if (hasAlt)
            {
                // Alt+F5: 生成不同类型的伪数据
                //m_renderManager.genAlternativeFakeData(); // 假设存在此方法
                qDebug() << "GenAlternativeFakeData (Alt+F5)";
            }
            else if (hasShift)
            {
                // Shift+F5: 重新生成相同数量的伪数据
                //m_renderManager.regenerateFakeData(); // 假设存在此方法
                qDebug() << "RegenerateFakeData (Shift+F5)";
            }
            else
            {
                // 单独按F5: 默认行为
                genFakeData();
                qDebug() << "GenFakeData (F5)";
            }
            update();
        }
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
            m_posLast = event->pos();
            m_bDragging = true;
            setCursor(Qt::ClosedHandCursor);
        }
        QOpenGLWidget::mousePressEvent(event);
    }

    void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::MiddleButton)
        {
            m_bDragging = false;
            setCursor(Qt::ArrowCursor);
        }
        QOpenGLWidget::mouseReleaseEvent(event);
    }

    void RenderWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if (m_mouseCb)
        {
            QPointF worldPos = m_camera.screenToWorld(event->pos(), size());
            float x = worldPos.x();
            float y = worldPos.y();
            m_mouseCb(x, y);
        }

        if (m_bDragging && m_camera.isEnableTrans())
        {
            QPointF delta = event->pos() - m_posLast;
            m_camera.translate(delta, size());
            m_posLast = event->pos();
            update();
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

        float dScaleFactor = event->angleDelta().y() > 0 ? 1.1f : 0.9f;
        m_camera.scale(event->position(), dScaleFactor, size());
        update();
        QOpenGLWidget::wheelEvent(event);
    }

    void RenderWidget::setBackgroundColor(const float r, const float g, const float b)
    {
        m_renderManager.setBackgroundColor(Brush(r, g, b));
        update();
    }

    void RenderWidget::setAntiAliasEnabled(bool bEnabled)
    {
        if (m_bAntiAlias != bEnabled)
        {
            m_bAntiAlias = bEnabled;

            makeCurrent();
            m_bAntiAlias ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
            doneCurrent();

            update();
        }
    }

    bool RenderWidget::isAntiAliasEnabled() const
    {
        return m_bAntiAlias;
    }

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
        auto board = static_cast<CheckerboardRenderer*>(m_renderManager.getCheckerboardRenderer());
        if (board) { // 添加空指针检查
            board->setVisible(b);
            update();
        }
    }

    void RenderWidget::setCheckerboardSz(int n)
    {
        auto board = static_cast<CheckerboardRenderer*>(m_renderManager.getCheckerboardRenderer());
        if (board) { // 添加空指针检查
            board->setSize(static_cast<float>(n));
            update();
        }
    }

    void RenderWidget::setShowCheckerboardColor(Brush brushA, Brush brushB)
    {
        auto board = static_cast<CheckerboardRenderer*>(m_renderManager.getCheckerboardRenderer());
        if (board) { // 添加空指针检查
            board->setColors(brushA.getColor(), brushB.getColor());
            update();
        }
    }

    void RenderWidget::zoomToRange(float minX, float minY, float maxX, float maxY)
    {
        m_camera.zoomToRange(minX, minY, maxX, maxY, size());
        update();
    }

    void RenderWidget::clear()
    {
        makeCurrent();
        m_renderManager.cleanup();
        m_renderManager.initialize(this);
        m_camera = Camera();
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

        GLuint fbo, colorTex, depthRb;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sz.width(), sz.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        glGenRenderbuffers(1, &depthRb);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sz.width(), sz.height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            qWarning() << "error: FBO not complete";
            glDeleteRenderbuffers(1, &depthRb);
            glDeleteTextures(1, &colorTex);
            glDeleteFramebuffers(1, &fbo);
            doneCurrent();
            return;
        }

        glViewport(0, 0, sz.width(), sz.height());
        const Brush& b = m_renderManager.getBackgroundColor();
        glClearColor(b.r(), b.g(), b.b(), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_renderManager.render(m_camera.getMatrix());

        QImage img(sz, QImage::Format_ARGB32);
        glReadPixels(0, 0, sz.width(), sz.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
        img = img.mirrored(false, true);

        QPointF worldTopLeft = m_camera.screenToWorld(QPointF(0, 0), sz);
        QPointF worldBottomRight = m_camera.screenToWorld(QPointF(sz.width(), sz.height()), sz);
        QRectF worldRect(worldTopLeft, worldBottomRight);

        if (cb)
            cb({ img, worldRect });

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteRenderbuffers(1, &depthRb);
        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);

        doneCurrent();
    }

    void RenderWidget::genFakeData()
    {
        if (!m_dataGen)
        {
            m_dataGen = std::make_unique<FakeDataProvider>();
            m_dataGen->initialize();
        }

        static bool b = true;
        b = !b;
        auto triRenderer = static_cast<TriangleRenderer*>(m_renderManager.getTriangleRenderer());
        triRenderer->setBlendEnabled(b);

        auto boardRenderer = static_cast<CheckerboardRenderer*>(m_renderManager.getCheckerboardRenderer());
        boardRenderer->setVisible(true);

        // 线段数据
        if (0)
        {
            std::vector<PolylineData> vPLineDatas = m_dataGen->genLineData(60);
            auto lineRenderer = static_cast<LineRenderer*>(m_renderManager.getLineRenderer());
            lineRenderer->updateData(vPLineDatas);
        }

        // 三角形数据
        if (0)
        {
            std::vector<TriangleData> vTriDatas = m_dataGen->genTriangleData();
            auto triRenderer = static_cast<TriangleRenderer*>(m_renderManager.getTriangleRenderer());
            triRenderer->updateData(vTriDatas);
        }
        // 三角剖分数据
        if (1)
        {
            FakeTriangle fakeTriangle;
            fakeTriangle.generatePolygons(10, 3, 12, 0.5f);

            GLRhi::Brush brush(0.8f, 0.4f, 0.1f, 1.0f, 0.0f);
            GLRhi::TriangleData data = convertToTriangleData(fakeTriangle, 1, brush);
            triRenderer->updateData({ data });

        }

        // 纹理数据
        if (0)
        {
            std::vector<TextureData> vTexDatas = m_dataGen->genTextureData();
            auto texRenderer = static_cast<TextureRenderer*>(m_renderManager.getTextureRenderer());
            texRenderer->updateData(vTexDatas);
        }

        // 实例化纹理数据
        if (0)
        {
            GLuint textureArrayId = 0;
            int textureCount = 0;
            std::vector<InstanceTexData> vInstances = 
                m_dataGen->genInstanceTextureData(textureArrayId, textureCount);

            if (textureArrayId > 0 && !vInstances.empty())
            {
                auto instanceTexRenderer = static_cast<InstanceTextureRenderer*>(m_renderManager.getInstanceTextureRenderer());
                instanceTexRenderer->setTextureArray(textureArrayId, textureCount);
                instanceTexRenderer->updateInstances(vInstances);
            }
        }

        // 实例化线段数据
        if (0)
        {
            m_instanceLineFakeData->genLines(1000, 0.001f, 0.003f);
            std::vector<InstanceLineData>& lineData = m_instanceLineFakeData->getInstanceData();
            auto instanceLineRenderer = static_cast<InstanceLineRenderer*>(m_renderManager.getInstanceLineRenderer());
            instanceLineRenderer->updateInstances(lineData);
        }

        // 实例化三角形数据
        if (0)
        {
            m_instanceTriangleFakeData->genTriangles(1000, 0.02f, 0.050f);
            const auto& triangleData = m_instanceTriangleFakeData->getInstanceData();
            auto instanceTriangleRenderer = static_cast<InstanceTriangleRenderer*>(m_renderManager.getInstanceTriangleRenderer());
            instanceTriangleRenderer->updateInstances(triangleData);
        }
    }

    void RenderWidget::setMousePosCb(const GetMousePtCb& cb)
    {
        m_mouseCb = cb;
    }

    void RenderWidget::checkGLError(const QString& context)
    {
        GLenum err;
        if ((err = glGetError()) != GL_NO_ERROR)
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