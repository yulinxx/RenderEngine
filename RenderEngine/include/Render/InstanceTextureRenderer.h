#ifndef INSTANCE_TEXTURE_RENDERER_H
#define INSTANCE_TEXTURE_RENDERER_H

#include "Common/DllSet.h"
#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

namespace GLRhi
{
    class GLRENDER_API InstanceTextureRenderer final : public IRenderer
    {
    public:
        InstanceTextureRenderer() = default;
        ~InstanceTextureRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

        // 设置纹理数组和实例数据
        void setTextureArray(GLuint texArrayId, int layerCount);
        void updateInstances(const std::vector<InstanceTexData>& instances);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nQuadVbo = 0;

        GLuint m_texArray = 0;          // 纹理数组ID
        int    m_layerCount = 0;        // 纹理层数

        size_t m_nInstCount = 0;        // 实例数量

        // Uniform
        GLint m_uCameraMatLoc = -1;
        GLint m_uTexArrayLoc = -1;
    };
}

#endif  // INSTANCE_TEXTURE_RENDERER_H