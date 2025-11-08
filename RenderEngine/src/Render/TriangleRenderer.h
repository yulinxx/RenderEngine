

// TriangleRenderer.h
#ifndef TRIANGLE_RENDERER_H
#define TRIANGLE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>
#include <QOpenGLShaderProgram>

namespace GLRhi
{
    struct TriangleData
    {
        std::vector<float> vertices;       // 格式: x, y, len
        std::vector<unsigned int> indices; // 索引
        Brush brush;
    };

    class TriangleRenderer : public IRenderer
    {
    public:
        TriangleRenderer() = default;
        ~TriangleRenderer() override { cleanup(); }

        bool initialize(QOpenGLFunctions_3_3_Core *gl) override;
        void render(const float *cameraMat) override;
        void cleanup() override;
        void updateData(std::vector<TriangleData> &vTriDatas);

    private:
        // 批次信息（每个TriangleData对应一个批次）
        struct Batch
        {
            unsigned int indexOffset; // 索引缓冲区偏移（字节）
            unsigned int indexCount;  // 索引数量
            Brush brush;              // 该批次使用的画刷
        };

        QOpenGLFunctions_3_3_Core *m_gl = nullptr;
        QOpenGLShaderProgram *m_program = nullptr;

        // OpenGL 对象
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;

        // 数据存储
        std::vector<TriangleData> m_vTriDatas;
        std::vector<Batch> m_batches;
        float m_depth = 0.5f;

        // Uniform 位置
        GLint m_colorLoc = -1;
        GLint m_depthLoc = -1;
        GLint m_cameraMatLoc = -1;
    };
}

#endif // TRIANGLE_RENDERER_H