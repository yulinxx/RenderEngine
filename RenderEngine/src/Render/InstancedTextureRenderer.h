// InstancedTextureRenderer.h
#ifndef INSTANCED_TEXTURE_RENDERER_H
#define INSTANCED_TEXTURE_RENDERER_H

#include "Render/IRenderer.h"
#include <vector>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

namespace GLRhi
{
    /**
     * @struct InstancedTextureData
     * @brief 单个实例的数据（不包含几何体）
     */
    struct InstancedTextureData
    {
        float x, y;          // 位置
        float width, height; // 尺寸
        int   textureLayer;  // 纹理数组层索引
        float alpha;         // 透明度
        // 可扩展：rotation, color 等
    };

    class InstancedTextureRenderer : public IRenderer
    {
    public:
        InstancedTextureRenderer() = default;
        ~InstancedTextureRenderer() override { cleanup(); }

        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;
        
        // 设置纹理数组和实例数据
        void setTextureArray(GLuint textureArrayId, int layerCount);
        void updateInstances(const std::vector<InstancedTextureData>& instances);

    private:
        QOpenGLFunctions_3_3_Core* m_gl = nullptr;
        QOpenGLShaderProgram* m_program = nullptr;

        GLuint m_vao = 0, m_vbo = 0;          // 单位矩形VBO + 实例VBO
        GLuint m_textureArray = 0;            // 纹理数组ID
        int    m_layerCount = 0;              // 纹理层数

        GLint m_cameraMatLoc = -1;
        GLint m_texArrayLoc = -1;
        GLint m_alphaLoc = -1;

        size_t m_instanceCount = 0;           // 当前实例数量
    };
}

#endif