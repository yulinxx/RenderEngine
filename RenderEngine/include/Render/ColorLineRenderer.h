#ifndef COLORLINE_RENDERER_H
#define COLORLINE_RENDERER_H

#include "Common/DllSet.h"
#include "IRenderer.h"
#include "RenderCommon.h"
#include "Render/LineRenderer.h"

#include <vector>

namespace GLRhi
{
    class GLRENDER_API ColorLineRenderer : public IRenderer
    {
    public:
        ColorLineRenderer() = default;
        ~ColorLineRenderer() override
        {
            cleanup();
        }

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        void updateData(const std::vector<PolylineData>& polylines);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<PolylineData> m_vPlineInfos;
        std::vector<GLuint> m_vIndices;

        // Uniform
        int m_uCameraMatLoc = -1; // 相机矩阵Uniform位置
        int m_uColorLoc = -1;     // 颜色Uniform位置
        int m_uDepthLoc = -1;     // 深度Uniform位置
    };
}
#endif // COLORLINE_RENDERER_H
