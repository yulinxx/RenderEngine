#ifndef LINE_RENDERER_H
#define LINE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>
#include "DataManager/PolylinesVboManager.h"

namespace GLRhi
{
    class GLRENDER_API LineRenderer : public IRenderer
    {
    public:
        LineRenderer();
        ~LineRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        void clearData() override;

        void updateData(const std::vector<PolylineData>& vPolylineDatas);
        void addPolyline(long long id, const float* verts, size_t n, float r, float g, float b);


    private:
        QMatrix3x3 m_mat;

        // Uniform
        int m_uCameraMatLoc = -1;
        int m_uColorLoc = -1;
        int m_uDepthLoc = -1;

        PolylinesVboManager m_lineBuffer;
    };
}
#endif // LINE_RENDERER_H