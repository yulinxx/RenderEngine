#ifndef INSTANCE_TRIANGLE_RENDERER_H
#define INSTANCE_TRIANGLE_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    class GLRENDER_EXPORT InstanceTriangleRenderer : public IRenderer
    {
    public:
        InstanceTriangleRenderer() = default;
        ~InstanceTriangleRenderer() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        // 更新实例化三角形数据
        void updateInstances(const std::vector<InstanceTriangleData>& instances);

        // 设置是否启用颜色混合
        void setBlendEnabled(bool enabled);
        // 获取当前混合状态
        bool isBlendEnabled() const;

    private:
        GLuint m_nVao = 0;
        GLuint m_nVertexVbo = 0;  // 存储基础三角形顶点的VBO
        GLuint m_nInstanceVbo = 0; // 存储实例化数据的VBO

        size_t m_nInstanceCount = 0; // 实例数量

        bool m_bBlend = true; // 是否启用混合

        // Uniform
        GLint m_uCameraMatLoc = -1;
        GLint m_uDepthLoc = -1;
    };
}

#endif // INSTANCE_TRIANGLE_RENDERER_H