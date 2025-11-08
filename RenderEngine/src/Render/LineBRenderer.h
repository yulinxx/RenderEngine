#ifndef LINEBRENDER_H
#define LINEBRENDER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include <vector>

namespace GLRhi
{
    struct LineBInfo
    {
        GLuint vao = 0;
        GLuint vbo = 0;
        std::vector<float> vertices; // 格式: x, y, length (每个顶点3个float)
        size_t vertexCount = 0;
        Brush color;
        int lineType = 100;       // 线类型（实线/虚线等）
        float dashScale = 1.0f;   // 虚线比例
        float thickness = 0.003f; // 线宽
    };

    class LineBRenderer : public IRenderer
    {
    public:
        LineBRenderer() = default;
        ~LineBRenderer() override
        {
            cleanup();
        }

        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

        // 更新粗线数据
        void updateData(float* data, size_t count, const Brush& color,
            int lineType = 100, float dashScale = 1.0f, float thickness = 0.003f);

    private:
        std::vector<LineBInfo> m_lineBInfos;
        // Uniform变量位置
        int m_cameraMatLoc = -1;
        int m_colorLoc = -1;
        int m_lineTypeLoc = -1;
        int m_dashScaleLoc = -1;
        int m_thicknessLoc = -1;
        float m_depth = 0.4f; // 粗线默认深度
    };
}
#endif // LINEBRENDER_H