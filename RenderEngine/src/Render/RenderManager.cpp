#include "Render/RenderManager.h"
#include <QDebug>
#include <QFileInfo>
#include <cassert>
#include <memory>

#include "FakeData/FakePolyLineData.h"
#include "FakeData/FakeTriangleData.h"
#include "FakeData/FakeTextureData.h"
#include "Common/TextureLoader.h"

#include "Common/Tools.h"

namespace GLRhi
{
    RenderManager::RenderManager()
    {
        // 初始化默认背景色
        m_bgColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    }

    RenderManager::~RenderManager()
    {
        cleanup();
    }

    bool RenderManager::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"RenderManager initialize: OpenGL functions not available");
            return false;
        }

        // 初始化所有渲染器
        m_boardRenderer = std::make_unique<CheckerboardRenderer>();
        m_lineRenderer = std::make_unique<LineRenderer>();
        m_lineUBORenderer = std::make_unique<LineRendererUbo>();
        m_lineBRenderer = std::make_unique<LineBRenderer>();
        m_triRenderer = std::make_unique<TriangleRenderer>();
        m_imageRenderer = std::make_unique<ImageRenderer>();
        m_texRenderer = std::make_unique<TextureRenderer>();
        m_instancTexRenderer = std::make_unique<InstanceTextureRenderer>();
        m_instanceLineRenderer = std::make_unique<InstanceLineRenderer>();
        m_instanceTriangleRenderer = std::make_unique<InstanceTriangleRenderer>();

        // 初始化伪数据生成器
        m_instanceLineFakeData = std::make_unique<InstanceLineFakeData>();
        m_instanceTriangleFakeData = std::make_unique<InstanceTriangleFakeData>();

        bool success = true;
        success &= m_boardRenderer->initialize(m_gl);
        success &= m_lineRenderer->initialize(m_gl);
        success &= m_lineUBORenderer->initialize(m_gl);
        success &= m_lineBRenderer->initialize(m_gl);
        success &= m_triRenderer->initialize(m_gl);
        success &= m_imageRenderer->initialize(m_gl);
        success &= m_texRenderer->initialize(m_gl);
        success &= m_instancTexRenderer->initialize(m_gl);
        success &= m_instanceLineRenderer->initialize(m_gl);
        success &= m_instanceTriangleRenderer->initialize(m_gl);

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

        m_gl->glClearColor(m_bgColor.r(), m_bgColor.g(), m_bgColor.b(), m_bgColor.a());
        m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float* mat = cameraMat;
        if (!cameraMat)
        {
            float defaultMvpMatrix[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };

            mat = defaultMvpMatrix;
        }

        m_boardRenderer->render(mat);
        m_triRenderer->render(mat);
        m_lineRenderer->render(mat);
        m_lineUBORenderer->render(mat);
        m_lineBRenderer->render(mat);
        m_imageRenderer->render(mat);
        m_texRenderer->render(mat);
        m_instancTexRenderer->render(mat);
        m_instanceLineRenderer->render(mat);
        m_instanceTriangleRenderer->render(mat);
    }

    void RenderManager::cleanup()
    {
        m_boardRenderer->cleanup();
        m_lineRenderer->cleanup();
        m_lineUBORenderer->cleanup();
        m_lineBRenderer->cleanup();
        m_triRenderer->cleanup();
        m_imageRenderer->cleanup();
        m_texRenderer->cleanup();
        m_instancTexRenderer->cleanup();
        m_instanceLineRenderer->cleanup();
        m_instanceTriangleRenderer->cleanup();

        if (m_instanceLineFakeData)
        {
            m_instanceLineFakeData->clear();
            m_instanceLineFakeData = nullptr;
        }

        if (m_instanceTriangleFakeData)
        {
            m_instanceTriangleFakeData->clear();
            m_instanceTriangleFakeData = nullptr;
        }

        if (m_dataGen)
        {
            m_dataGen->cleanup();
            m_dataGen = nullptr;
        }

        m_gl = nullptr;
    }

    IRenderer* RenderManager::getCheckerboardRenderer()
    {
        return m_boardRenderer.get();
    }

    IRenderer* RenderManager::getLineRenderer()
    {
        return m_lineRenderer.get();
    }

    IRenderer* RenderManager::getLineUboRenderer()
    {
        return m_lineUBORenderer.get();
    }

    IRenderer* RenderManager::getLineBRenderer()
    {
        return m_lineBRenderer.get();
    }

    IRenderer* RenderManager::getTriangleRenderer()
    {
        return m_triRenderer.get();
    }

    IRenderer* RenderManager::getImageRenderer()
    {
        return m_imageRenderer.get();
    }

    IRenderer* RenderManager::getTextureRenderer()
    {
        return m_texRenderer.get();
    }

    IRenderer* RenderManager::getInstanceTextureRenderer()
    {
        return m_instancTexRenderer.get();
    }

    IRenderer* RenderManager::getInstanceLineRenderer()
    {
        return m_instanceLineRenderer.get();
    }

    IRenderer* RenderManager::getInstanceTriangleRenderer()
    {
        return m_instanceTriangleRenderer.get();
    }

    void RenderManager::setBackgroundColor(const Brush& color)
    {
        m_bgColor = color;
    }

    const Brush& RenderManager::getBackgroundColor() const
    {
        return m_bgColor;
    }

    void RenderManager::genFakeData()
    {
        if (!m_dataGen)
        {
            m_dataGen = std::make_unique<FakeDataProvider>();
            m_dataGen->initialize();
        }

        static bool b = true;
        b = !b;
        static_cast<TriangleRenderer*>(m_triRenderer.get())->setBlendEnabled(b);

        static_cast<CheckerboardRenderer*>(m_boardRenderer.get())->setVisible(true);

        // 线段数据
        if (1)
        {
            std::vector<PolylineData> vPLineDatas = m_dataGen->genLineData(60);

            static_cast<LineRendererUbo*>(m_lineUBORenderer.get())->updateData(vPLineDatas);

            m_dataManager.setPolylineDatas(vPLineDatas);
        }

        // 三角形数据
        if (1)
        {
            std::vector<TriangleData> vTriDatas = m_dataGen->genTriangleData();
            static_cast<TriangleRenderer*>(m_triRenderer.get())->updateData(vTriDatas);

            m_dataManager.setTriangleDatas(vTriDatas);
        }

        // 纹理数据
        if (1)
        {
            std::vector<TextureData> vTexDatas = m_dataGen->genTextureData();
            static_cast<TextureRenderer*>(m_texRenderer.get())->updateData(vTexDatas);
            m_dataManager.setTextureDatas(vTexDatas);
        }

        // 实例化纹理数据
        if (1)
        {
            GLuint textureArrayId = 0;
            int textureCount = 0;
            std::vector<InstanceTexData> vInstances =
                m_dataGen->genInstanceTextureData(textureArrayId, textureCount);

            if (textureArrayId > 0 && !vInstances.empty())
            {
                static_cast<InstanceTextureRenderer*>(m_instancTexRenderer.get())
                    ->setTextureArray(textureArrayId, textureCount);

                static_cast<InstanceTextureRenderer*>(m_instancTexRenderer.get())->updateInstances(vInstances);

                m_dataManager.setInstanceTextureDatas(vInstances);
            }
        }

        // 实例化线段数据
        if (1)
        {
            m_instanceLineFakeData->genLines(1000, 0.001f, 0.003f);
            std::vector<InstanceLineData>& lineData = m_instanceLineFakeData->getInstanceData();
            static_cast<InstanceLineRenderer*>(m_instanceLineRenderer.get())->updateInstances(lineData);

            m_dataManager.setInstanceLineDatas(lineData);
        }

        // 实例化三角形数据
        if (1)
        {
            m_instanceTriangleFakeData->genTriangles(1000, 0.02f, 0.050f);
            const auto& triangleData = m_instanceTriangleFakeData->getInstanceData();
            static_cast<InstanceTriangleRenderer*>(m_instanceTriangleRenderer.get())->updateInstances(triangleData);
        }
    }

    void RenderManager::dataCRUD()
    {
        m_dataManager.setLineDatasCRUD();
    }

}