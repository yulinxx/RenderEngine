#ifndef TRIANGLE_RENDERER_H
#define TRIANGLE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>

namespace GLRhi
{
    class GLRENDER_API TriangleRenderer : public IRenderer
    {
    public:
        TriangleRenderer() = default;
        ~TriangleRenderer() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        // 更新三角形数据
        void updateData(const std::vector<TriangleData>& triangleDatas);

        // 设置是否启用颜色混合
        void setBlendEnabled(bool enabled);
        // 获取当前混合状态
        bool isBlendEnabled() const;

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<Brush> m_vTriangleBrush;    // 每个三角形组的颜色信息
        std::vector<GLuint> m_vIndexCounts;     // 每个三角形组的索引数
        std::vector<GLuint> m_vIndexOffsets;    // 索引偏移量

        bool m_bBlend = true;  // 是否启用混合

        // Uniform locations
        int m_uCameraMatLoc = -1;
        int m_uColorLoc = -1;
        int m_uDepthLoc = -1;
    };
}

#endif // TRIANGLE_RENDERER_H
