#ifndef COLORFILLRENDERER_H
#define COLORFILLRENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include "TriangleRenderer.h"

#include <vector>

namespace GLRhi
{
    // struct TriangleInfo
    // {
    //     GLuint vao = 0;
    //     GLuint vbo = 0;
    //     std::vector<float> vertices; // 格式: x, y, depth (每个顶点3个float)
    //     size_t vertexCount = 0;
    //     Brush color;
    // };

    class ColorFillRenderer : public IRenderer
    {
    public:
        ColorFillRenderer() = default;
        ~ColorFillRenderer() override
        {
            cleanup();
        }

        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

        // 更新填充数据（需按三角形顶点顺序传入）
        void updateData(float* data, size_t count, const Brush& color);

    private:
        std::vector<TriangleInfo> m_fillInfos;
        int m_cameraMatLoc = -1;  // 相机矩阵Uniform位置
        float m_depth = 0.5f;     // 填充默认深度
    };
}
#endif // COLORFILLRENDERER_H