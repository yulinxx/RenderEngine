#include "RenderManager.h"
#include <QDebug>
#include <cassert>
#include <memory>

#include "FakeData/FakePolyLineData.h"
#include "FakeData/FakeTriangleData.h"

#include "Common/Tools.h"

namespace GLRhi
{
    RenderManager::RenderManager()
    {
        // 初始化默认背景色
        m_bgColor = { 1.0f, 1.0f, 0.0f, 1.0f };

        // 使用std::unique_ptr替代原始指针，实现自动内存管理
        m_boardRenderer = std::make_unique<CheckerboardRenderer>();
        m_lineRenderer = std::make_unique<LineRenderer>();
        //m_lineRenderer = std::make_unique<LineRendererUbo>();
        m_lineBRenderer = std::make_unique<LineBRenderer>();
        m_triRenderer = std::make_unique<TriangleRenderer>();
        m_imageRenderer = std::make_unique<ImageRenderer>();
    }

    RenderManager::~RenderManager()
    {
        cleanup();
        // std::unique_ptr会自动管理内存，无需手动释放
    }

    bool RenderManager::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            qWarning() << "RenderManager::initialize: OpenGL functions not available";
            assert(!"RenderManager initialize: OpenGL functions not available");

            return false;
        }

        // 初始化所有渲染器
        bool success = true;
        success &= m_boardRenderer->initialize(m_gl);
        success &= m_lineRenderer->initialize(m_gl);
        success &= m_lineBRenderer->initialize(m_gl);
        success &= m_triRenderer->initialize(m_gl);
        success &= m_imageRenderer->initialize(m_gl);

        if (!success)
        {
            qWarning() << "RenderManager::initialize: Failed to initialize one or more renderers";
            return false;
        }

        genFakeData();

        return true;
    }

    void RenderManager::render(const float* cameraMat)
    {
        if (!m_gl)
            return;

        // 设置背景色
        m_gl->glClearColor(m_bgColor.r(), m_bgColor.g(), m_bgColor.b(), m_bgColor.a());
        m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //float mat[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr float mat[9] = {
        2.0f / 879.0f, 0.0f, 0.0f,
        0.0f, 2.0f / 1368.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        };

        m_boardRenderer->render(mat);
        m_triRenderer->render(mat);
        m_lineRenderer->render(mat);
        m_lineBRenderer->render(mat);
        m_imageRenderer->render(mat);
    }

    void RenderManager::cleanup()
    {
        m_boardRenderer->cleanup();
        m_lineRenderer->cleanup();
        m_lineBRenderer->cleanup();
        m_triRenderer->cleanup();
        m_imageRenderer->cleanup();

        m_gl = nullptr;
    }

    void RenderManager::genFakeData()
    {
        // Line
        if (0)
        {
            std::vector<PolylineData> vPLineDatas;

            FakePolyLineData fakePolyLineData;
            for (size_t i = 0; i < 20; ++i) // Group
                //for (size_t i = 0; i < 2; ++i) // Group
            {
                // fakePolyLineData.setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);

                fakePolyLineData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
                fakePolyLineData.generateLines(100000, 2, 15);

                std::vector<float> datas = fakePolyLineData.getVertices();
                std::vector<size_t> lineVertexCounts = fakePolyLineData.getLineInfos();

                float r = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float g = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float b = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float d = fakePolyLineData.getRandomFloat(-1.0f, 1.0f);

                PolylineData polyLineData{ datas, lineVertexCounts, {r, g, b, 1.0, d} };
                //PolylineData polyLineData{ datas, lineVertexCounts, {r, 0.0, 0.0, 1.0, d} };

                vPLineDatas.emplace_back(polyLineData);
            }

            //static_cast<LineRendererUbo*>(m_lineRenderer.get())->updateData(vPLineDatas);
            static_cast<LineRenderer*>(m_lineRenderer.get())->updateData(vPLineDatas);
        }

        // Triangle
        if (0)
        {
            // 准备数据
            std::vector<GLRhi::TriangleData> vTriDatas(2);

            // 第一个三角形批次
            vTriDatas[0].vertices = { 0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f };
            vTriDatas[0].indices = { 0, 1, 2 };
            vTriDatas[0].brush = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0 };

            // 第二个三角形批次
            vTriDatas[1].vertices = { 2.0f, 0.0f, 0.0f,  3.0f, 0.0f, 0.0f,  2.0f, 1.0f, 0.0f };
            vTriDatas[1].indices = { 0, 1, 2 };
            vTriDatas[1].brush = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0 };

            static_cast<TriangleRenderer*>(m_triRenderer.get())->updateData(vTriDatas);
        }

        if (1)
        {
            std::vector<TriangleData> vTriDatas;
            vTriDatas.reserve(10);

            for (size_t i = 0; i < 100; ++i) // Group
            {
                FakeTriangleData fakeTriangleData;
                // fakeTriangleData.setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);
                fakeTriangleData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
                fakeTriangleData.generateTriangles(10000);

                TriangleData triData{};
                triData.vertices = fakeTriangleData.getVertices();
                triData.indices = fakeTriangleData.getIndices();

                float r = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float g = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float b = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float a = fakeTriangleData.getRandomFloat(0.3f, 1.0f);
                float d = fakeTriangleData.getRandomFloat(-1.0f, 1.0f);
                triData.brush = { r, g, b, a, 0.0f, 0 };

                vTriDatas.push_back(triData);
            }
            static_cast<TriangleRenderer*>(m_triRenderer.get())->updateData(vTriDatas);
        }
    }
}