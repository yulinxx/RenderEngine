#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include "IRenderer.h"
#include "CheckerboardRenderer.h"
#include "LineRenderer.h"
#include "LineTestRenderer.h"
#include "LineRendererUbo.h"
#include "LineBRenderer.h"
#include "TriangleRenderer.h"
#include "ImageRenderer.h"
#include "TextureRenderer.h"
#include "InstanceTextureRenderer.h"
#include "InstanceLineRenderer.h"
#include "InstanceTriangleRenderer.h"
#include "RenderCommon.h"

#include "Common/DllSet.h"
#include "Render/RenderDataManager.h"

// #include "FakeData/FakeDataProvider.h"
// #include "FakeData/InstanceLineFakeData.h"
// #include "FakeData/InstanceTriangleFakeData.h"
#include <QOpenGLFunctions_3_3_Core>

#include <memory>

namespace GLRhi
{
    class GLRENDER_API RenderManager final
    {
    public:
        RenderManager();
        ~RenderManager();

    public:
        // 初始化所有渲染器
        bool initialize(QOpenGLContext* context);

        // 渲染
        void render(const float* cameraMat);

        // 清理所有渲染器
        void cleanup();

        // 渲染器
        IRenderer* getCheckerboardRenderer();
        IRenderer* getLineRenderer();
        IRenderer* getLineUboRenderer();
        IRenderer* getLineBRenderer();
        IRenderer* getTriangleRenderer();
        IRenderer* getImageRenderer();
        IRenderer* getTextureRenderer();
        IRenderer* getInstanceTextureRenderer();
        IRenderer* getInstanceLineRenderer();
        IRenderer* getInstanceTriangleRenderer();

        // 背景色设置
        void setBackgroundColor(const Brush& color);
        const Brush& getBackgroundColor() const;

        void dataCRUD();

    private:
        QOpenGLFunctions_3_3_Core* m_gl{ nullptr };  // OpenGL函数指针
        QOpenGLContext* m_context{ nullptr };
        Brush m_bgColor{ 1.0, 1.0, 0.0, -1.0 }; // 背景色

        // 渲染器
        std::unique_ptr<IRenderer> m_boardRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_lineRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_lineUBORenderer{ nullptr };
        std::unique_ptr<IRenderer> m_lineBRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_triRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_imageRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_texRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_instancTexRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_instanceLineRenderer{ nullptr };
        std::unique_ptr<IRenderer> m_instanceTriangleRenderer{ nullptr };

        // // 实例化伪数据生成器
        // std::unique_ptr<InstanceLineFakeData> m_instanceLineFakeData{ nullptr };
        // std::unique_ptr<InstanceTriangleFakeData> m_instanceTriangleFakeData{ nullptr };

        // // 数据生成器
        // std::unique_ptr<FakeDataProvider> m_dataGen{ nullptr };

        RenderDataManager m_dataManager;
    };
}
#endif // RENDERMANAGER_H