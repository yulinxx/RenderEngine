#ifndef IMAGE_RENDERER_H
#define IMAGE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include <vector>

namespace GLRhi
{
    struct ImageInfoEx
    {
        GLuint textureId = 0;           // 纹理ID
        std::vector<float> verts;    // x, y, u, v
        size_t count = 0;         // 顶点数量
        float depth = 0.0f;             // 深度值
    };

    class GLRENDER_API ImageRenderer : public IRenderer
    {
    public:
        ImageRenderer() = default;
        ~ImageRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

        // 更新图像数据（顶点+纹理+深度）
        void updateData(const float* verts, size_t count,
            const unsigned char* imgData, int width, int height,
            float depth = 0.5f);

    private:
        std::vector<ImageInfoEx> m_vImageInfos;
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;

        float m_dDepth = 0.1f;     // 图像默认深度

        // Uniform
        int m_uCameraMatLoc = -1; // 相机矩阵Uniform位置
        int m_uTextureLoc = -1;   // 纹理Uniform位置
        int m_uDepthLoc = -1;     // 深度Uniform位置
    };
}
#endif // IMAGE_RENDERER_H