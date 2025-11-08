#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include "IRenderer.h"
#include "CheckerboardRenderer.h"
#include "LineRenderer.h"
#include "LineRendererUbo.h"
#include "LineBRenderer.h"
#include "TriangleRenderer.h"
#include "ImageRenderer.h"
#include "RenderCommon.h"

#include <QOpenGLFunctions_3_3_Core>
#include <memory>

namespace GLRhi
{
    class RenderManager final
    {
    public:
        RenderManager();
        ~RenderManager();

    public:
        // 初始化所有渲染器
        bool initialize(QOpenGLFunctions_3_3_Core* gl);

        // 执行所有渲染器的渲染
        void render(const float* cameraMat);

        // 清理所有渲染器资源
        void cleanup();

        // 各渲染器接口
        IRenderer* getCheckerboardRenderer()
        {
            return m_boardRenderer.get();
        }
        IRenderer* getLineRenderer()
        {
            return m_lineRenderer.get();
        }
        IRenderer* getLineBRenderer()
        {
            return m_lineBRenderer.get();
        }
        IRenderer* geTriangleRenderer()
        {
            return m_triRenderer.get();
        }
        IRenderer* getImageRenderer()
        {
            return m_imageRenderer.get();
        }

        // 背景色设置
        void setBackgroundColor(const Brush& color)
        {
            m_bgColor = color;
        }
        const Brush& getBackgroundColor() const
        {
            return m_bgColor;
        }

        void genFakeData();

    private:
        QOpenGLFunctions_3_3_Core* m_gl{ nullptr };  // OpenGL函数指针
        Brush m_bgColor{ 1.0, 1.0, 0.0, -1.0 }; // 背景色

        // 各类型渲染器实例，使用std::unique_ptr实现自动内存管理
        std::unique_ptr<IRenderer> m_boardRenderer;
        std::unique_ptr<IRenderer> m_lineRenderer;
        std::unique_ptr<IRenderer> m_lineBRenderer;
        std::unique_ptr<IRenderer> m_triRenderer;
        std::unique_ptr<IRenderer> m_imageRenderer;
    };
}
#endif // RENDERMANAGER_H