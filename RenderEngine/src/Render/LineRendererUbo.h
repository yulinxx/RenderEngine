// ============= LineRendererUbo.h =============
#ifndef LINE_RENDERER_UBO_H
#define LINE_RENDERER_UBO_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include "Render/LineRenderer.h"
#include <vector>

namespace GLRhi
{
    class LineRendererUbo : public IRenderer
    {
    public:
        LineRendererUbo() = default;
        ~LineRendererUbo() override
        {
            cleanup();
        }

        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;
        void updateData(const std::vector<PolylineData>& polylines);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;
        GLuint m_nUbo = 0;

        int m_cameraMatLoc = -1;

        struct BatchInfo
        {
            GLuint indexCount;
            GLuint indexOffset;
            GLuint lineDataIndex; // 关联到哪一个PolylineData
        };

        std::vector<BatchInfo> m_batches;
        GLsizei m_totalIndexCount = 0;

    };
}
#endif  // LINE_RENDERER_UBO_H