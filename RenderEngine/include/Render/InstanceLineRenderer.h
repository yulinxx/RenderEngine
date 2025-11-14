#ifndef INSTANCE_LINE_RENDERER_H
#define INSTANCE_LINE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    // 实例化线段数据结构
    struct InstanceLineData
    {
        float pos1[3];    // 线段第一个端点位置
        float pos2[3];    // 线段第二个端点位置
        float color[4];   // 线段颜色 (RGBA)
        float width;      // 线段宽度
        float depth;      // 深度值
    };

    class GLRENDER_EXPORT InstanceLineRenderer : public IRenderer
    {
    public:
        InstanceLineRenderer() = default;
        ~InstanceLineRenderer() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
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