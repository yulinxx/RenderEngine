#ifndef TEXTURE_RENDERER_H
#define TEXTURE_RENDERER_H

#include "Render/IRenderer.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    struct TextureData
    {
        std::vector<float> vertices; // 格式: x, y, u, v (位置 + 纹理坐标)
        std::vector<unsigned int> indices;
        GLuint textureId;
        Brush brush;
    };

    class TextureRenderer : public IRenderer
    {
    public:
        TextureRenderer() = default;
        ~TextureRenderer() override
        {
            cleanup();
        }

    public:

        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;
        void updateData(const std::vector<TextureData>& vTexDatas);

    private:
        struct Batch
        {
            unsigned int indexOffset;   // 索引偏移（元素个数）
            unsigned int indexCount;
            GLuint textureId;           // 该批次使用的纹理
            Brush brush;
        };

        QOpenGLFunctions_3_3_Core* m_gl = nullptr;
        QOpenGLShaderProgram* m_program = nullptr;

        GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
        std::vector<Batch> m_batches;
        std::vector<GLuint> m_textureIds; // 存储纹理ID以在清理时释放

        GLint m_texLoc = -1;
        GLint m_alphaLoc = -1;
        GLint m_cameraMatLoc = -1;
    };
}

#endif