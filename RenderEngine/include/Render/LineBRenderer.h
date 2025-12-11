#ifndef LINEBRENDER_H
#define LINEBRENDER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include <vector>

namespace GLRhi
{
    struct LineBInfoEx
    {
        GLuint vao = 0;
        GLuint vbo = 0;
        std::vector<float> verts; // 格式: x, y, length (每个顶点3个float)
        size_t count = 0;
        Brush color;
        int lineType = 100;       // 线类型（实线/虚线等）
        float dashScale = 1.0f;   // 虚线比例
        float thickness = 0.003f; // 线宽
    };

    class GLRENDER_API LineBRenderer : public IRenderer
    {
    public:
        LineBRenderer() = default;
        ~LineBRenderer() override;

        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

        // 更新粗线数据
        void updateData(float* data, size_t count, const Brush& color,
            int lineType = 100, float dashScale = 1.0f, float thickness = 0.003f);

    private:
        std::vector<LineBInfoEx> m_lineBInfos;

        float m_dDepth = 0.4f;

        // Uniform
        int m_uCameraMatLoc = -1;
        int m_uColorLoc = -1;
        int m_uLineTypeLoc = -1;
        int m_uDashScaleLoc = -1;
        int m_uThicknessLoc = -1;
        int m_uDepthLoc = -1;
    };
}
#endif // LINEBRENDER_H