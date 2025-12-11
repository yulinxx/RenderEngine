#ifndef TRIANGLE_RENDERER_H
#define TRIANGLE_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    class GLRENDER_API TriangleRenderer : public IRenderer
    {
    public:
        TriangleRenderer() = default;
        ~TriangleRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        void updateData(const std::vector<TriangleData>& vTriDatas);

        // 是否启用颜色混合
        void setBlendEnabled(bool enabled);
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