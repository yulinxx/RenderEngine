#ifndef TEXTURE_RENDERER_H
#define TEXTURE_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"

#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    class GLRENDER_API TextureRenderer : public IRenderer
    {
    public:
        TextureRenderer() = default;
        ~TextureRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;
        void updateData(const std::vector<TextureData>& vTexDatas);

    private:
        struct Batch
        {
            unsigned int indexOffset;   // 索引偏移（元素个数）
            unsigned int indexCount;
            GLuint tex;                 // 纹理
            Brush brush;                // 画刷
        };

        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<Batch> m_vBatches;
        std::vector<GLuint> m_vTextureIds; // 存储纹理ID以在清理时释放

        // Uniform
        GLint m_uCameraMatLoc = -1;
        GLint m_uDepthLoc = -1;
        GLint m_uTexLoc = -1;
        GLint m_uAlphaLoc = -1;
    };
}

#endif // TEXTURE_RENDERER_H