#ifndef COLOR_TRIANGLE_RENDERER_H
#define COLOR_TRIANGLE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include "Render/TriangleRenderer.h"

#include <vector>

namespace GLRhi
{
    class GLRENDER_API ColorTriangleRenderer : public IRenderer
    {
    public:
        ColorTriangleRenderer() = default;
        ~ColorTriangleRenderer() override
        {
            cleanup();
        }

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

        // 更新填充数据
        void updateData(float* data, size_t count, const Brush& color);

    private:
        std::vector<TriangleData> m_vTriDatas;
        float m_dDepth = 0.2f;       // 填充默认深度

        // Uniform
        int m_uCameraMatLoc = -1;   // 相机矩阵Uniform位置
        int m_uDepthLoc = -1;       // 深度Uniform位置
    };
}
#endif // COLOR_TRIANGLE_RENDERER_H