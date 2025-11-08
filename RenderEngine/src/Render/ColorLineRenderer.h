#ifndef COLORLINE_RENDERER_H
#define COLORLINE_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include "Render/LineRenderer.h"

#include <vector>
namespace GLRhi
{
    //struct PolylineData
    //{
    //    std::vector<float> vertices{}; //  x, y, len 
    //    Brush brush{ 0.0, 0.0, 0.0, 1.0, 0.0 };
    //};

    class ColorLineRenderer : public IRenderer
    {
    public:
        ColorLineRenderer() = default;
        ~ColorLineRenderer() override
        {
            cleanup();
        }

    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        void updateData(float* data, size_t count, const Brush& color);
        void prepareBuffers(const std::vector<PolylineData>& polylines);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::vector<PolylineData> m_vPlineInfos;
        std::vector<GLuint> m_vIndices;

        //int m_cameraMatLoc = -1;
    };

}
#endif // COLORLINE_RENDERER_H
