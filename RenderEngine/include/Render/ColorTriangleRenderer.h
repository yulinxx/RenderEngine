#ifndef COLOR_TRIANGLE_RENDERER_H
#define COLOR_TRIANGLE_RENDERER_H

#include "Common/DllSet.h"
#include "IRenderer.h"
#include "RenderCommon.h"

#include "Render/TriangleRenderer.h"

#include <vector>

namespace GLRhi
{
    //struct TriangleData
    //{
    //    GLuint vao = 0;
    //    GLuint vbo = 0;
    //    std::vector<float> vertices; // 每个顶点: x, y, depth
    //    size_t vertexCount = 0;
    //    Brush brush{ 0.0, 0.0, 0.0, 1.0, 0.0 };
    //};

    class GLRENDER_API ColorTriangleRenderer : public IRenderer
    {
    public:
        ColorTriangleRenderer() = default;
        ~ColorTriangleRenderer() override
        {
            cleanup();
        }

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

        // 更新填充数据（需按三角形顶点顺序传入）
        void updateData(float* data, size_t count, const Brush& color);

    private:
        std::vector<TriangleData> m_vTriDatas;
        float m_dDepth = 0.5f;       // 填充默认深度

        // Uniform
        int m_uCameraMatLoc = -1;   // 相机矩阵Uniform位置
        int m_uDepthLoc = -1;       // 深度Uniform位置
    };
}
#endif // COLOR_TRIANGLE_RENDERER_H