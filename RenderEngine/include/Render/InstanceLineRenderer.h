#ifndef INSTANCE_LINE_RENDERER_H
#define INSTANCE_LINE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    class GLRENDER_API InstanceLineRenderer : public IRenderer
    {
    public:
        InstanceLineRenderer() = default;
        ~InstanceLineRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        // 更新实例化线段数据
        void updateInstances(const std::vector<InstanceLineData>& vData);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVertexVbo = 0;  // 存储基础线段顶点的VBO
        GLuint m_nInstanceVbo = 0; // 存储实例化数据的VBO

        size_t m_nInstanceCount = 0; // 实例数量

        // Uniform
        GLint m_uCameraMatLoc = -1;
    };
}

#endif // INSTANCE_LINE_RENDERER_H