#ifndef LINE_RENDERER_H
#define LINE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>

namespace GLRhi
{
    class GLRENDER_EXPORT LineRenderer : public IRenderer
    {
    public:
        LineRenderer() = default;
        ~LineRenderer() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        void updateData(const std::vector<PolylineData>& vPolylineDatas);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<Brush> m_vPlineBrush;       // 每类线的颜色信息
        std::vector<GLuint> m_vIndexCounts;     // 每条线段顶点数
        std::vector<GLuint> m_vIndexOffsets;    // 索引偏移量

        // Uniform
        int m_uCameraMatLoc = -1;
        int m_uColorLoc = -1;
        int m_uDepthLoc = -1;
    };
}
#endif // LINE_RENDERER_H