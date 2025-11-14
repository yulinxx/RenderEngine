#ifndef TRIANGLE_RENDERER_H
#define TRIANGLE_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    class GLRENDER_EXPORT TriangleRenderer : public IRenderer
    {
    public:
        TriangleRenderer() = default;
        ~TriangleRenderer() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        void updateData(std::vector<TriangleData>& vTriDatas);

        // 设置是否启用颜色混合
        void setBlendEnabled(bool enabled);
        // 获取当前混合状态
        bool isBlendEnabled() const;

    public:
    private:
        struct Batch
        {
            unsigned int indexOffset; // 索引缓冲区偏移（字节）
            unsigned int indexCount;  // 索引数量
            Brush brush;
        };

        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<Batch> m_vecBatches;

        bool m_bBlend = true;

        // Uniform
        GLint m_uCameraMatLoc = -1;
        GLint m_uColorLoc = -1;
        GLint m_uDepthLoc = -1;
    };
}

#endif // TRIANGLE_RENDERER_H