#include "Render/RenderManager.h"
#include <QDebug>
#include <QFileInfo>
#include <cassert>
#include <memory>

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

        // 移除了genFakeData调用，因为这个方法已经被移除

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

        // 移除了FakeData相关的清理代码，因为这些变量已经被移除

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

    void RenderManager::dataCRUD()
    {
        m_dataManager.setLineDatasCRUD();
    }
}