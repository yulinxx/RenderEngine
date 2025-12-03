#ifndef LINE_RENDERER_UBO_H
#define LINE_RENDERER_UBO_H

#include "Common/DllSet.h"
#include "IRenderer.h"
#include "RenderCommon.h"
#include "Render/LineRenderer.h"

#include <vector>

namespace GLRhi
{
    class GLRENDER_API LineRendererUbo : public IRenderer
    {
    public:
        LineRendererUbo() = default;
        ~LineRendererUbo() override;

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        void updateData(const std::vector<PolylineData>& polylines);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;
        GLuint m_nUbo = 0;

        struct BatchInfo
        {
            GLuint indexCount;
            GLuint indexOffset;
            GLuint lineDataIndex;
        };

        std::vector<BatchInfo> m_vecBatches;
        GLsizei m_totalIndexCount = 0;

        // Uniform
        int m_uCameraMatLoc = -1;
    };
}
#endif  // LINE_RENDERER_UBO_H