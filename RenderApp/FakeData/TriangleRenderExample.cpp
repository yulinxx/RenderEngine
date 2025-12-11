// TriangleRenderExample.cpp
// 使用 FakeTriangle 和 TriangleRenderer

#include "Common/Brush.h"
#include "FakeData/fakeTriangleMesh.h"
#include "FakeData/fakeTriangleMeshHelper.h"
#include "Render/TriangleRenderer.h"

// 示例：使用 FakeTriangle 和 TriangleRenderer 渲染随机多边形
class TriangleRenderExample
{
public:
    void initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "LineRenderer::initialize: context is null");
            return;
        }


        // 1. 初始化渲染器
        m_triangleRenderer.initialize(context);

        // 2. 生成随机多边形数据
        FakeTriangleMesh fakeTriangle;
        fakeTriangle.setRange(-1.0f, 1.0f, -1.0f, 1.0f);

        // 生成10个多边形，每个3-10个顶点，50%为凹多边形
        fakeTriangle.generatePolygons(10, 3, 10, 0.5f);

        // 3. 转换为渲染数据
        // 创建一个橙色画刷
        GLRhi::Brush orangeBrush(1.0f, 0.5f, 0.0f, 1.0f, 0.0f); // r, g, b, a, depth

        GLRhi::TriangleData triangleData = convertToTriangleData(fakeTriangle, 1, orangeBrush);

        // 4. 更新渲染器数据
        std::vector<GLRhi::TriangleData> triangleDatas = { triangleData };
        m_triangleRenderer.updateData(triangleDatas);
    }

    void render(float* cameraMat)
    {
        // 5. 渲染
        m_triangleRenderer.render(cameraMat);
    }

    void cleanup()
    {
        m_triangleRenderer.cleanup();
    }

private:
    GLRhi::TriangleRenderer m_triangleRenderer;
};

// 示例：生成多个不同颜色的多边形组
class MultiColorTriangleExample
{
public:
    void initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "LineRenderer::initialize: context is null");
            return;
        }

        m_triangleRenderer.initialize(context);

        std::vector<GLRhi::TriangleData> triangleDatas;

        // 生成3组不同颜色的多边形
        struct ColorConfig
        {
            GLRhi::Brush brush;
            size_t polygonCount;
            float concaveRatio;
        };

        std::vector<ColorConfig> configs = {
            { GLRhi::Brush(1.0f, 0.0f, 0.0f, 1.0f, 0.0f), 5, 0.2f },  // 红色，主要是凸多边形
            { GLRhi::Brush(0.0f, 1.0f, 0.0f, 1.0f, 0.1f), 5, 0.5f },  // 绿色，混合
            { GLRhi::Brush(0.0f, 0.0f, 1.0f, 1.0f, 0.2f), 5, 0.8f }   // 蓝色，主要是凹多边形
        };

        for (size_t i = 0; i < configs.size(); ++i)
        {
            FakeTriangleMesh fakeTriangle;
            fakeTriangle.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakeTriangle.generatePolygons(configs[i].polygonCount, 4, 8, configs[i].concaveRatio);

            GLRhi::TriangleData data = convertToTriangleData(fakeTriangle, static_cast<long long>(i), configs[i].brush);
            triangleDatas.push_back(data);
        }

        m_triangleRenderer.updateData(triangleDatas);
    }

    void render(float* cameraMat)
    {
        m_triangleRenderer.render(cameraMat);
    }

    void cleanup()
    {
        m_triangleRenderer.cleanup();
    }

private:
    GLRhi::TriangleRenderer m_triangleRenderer;
};

// 使用示例代码片段：

/*
    // 在渲染管理器或窗口类中使用：

    // 初始化阶段
    void MyRenderWindow::initializeGL()
    {
        QOpenGLFunctions_3_3_Core* gl = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

        // 创建并初始化三角形渲染器
        m_triangleRenderer = new GLRhi::TriangleRenderer();
        m_triangleRenderer->initialize(gl);

        // 生成测试数据
        FakeTriangle fakeTriangle;
        fakeTriangle.generatePolygons(20, 3, 12, 0.5f);

        // 转换并上传数据
        GLRhi::Brush brush(0.8f, 0.3f, 0.1f, 1.0f, 0.0f);
        GLRhi::TriangleData data = convertToTriangleData(fakeTriangle, 1, brush);
        m_triangleRenderer->updateData({data});
    }

    // 渲染阶段
    void MyRenderWindow::paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float cameraMat[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        m_triangleRenderer->render(cameraMat);
    }

    // 清理阶段
    void MyRenderWindow::cleanup()
    {
        if (m_triangleRenderer)
        {
            m_triangleRenderer->cleanup();
            delete m_triangleRenderer;
            m_triangleRenderer = nullptr;
        }
    }
    */