#ifndef LINE_TEST_RENDERER_H
#define LINE_TEST_RENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"
#include <vector>
#include <map>

namespace GLRhi
{
    class GLRENDER_API LineTestRenderer : public IRenderer
    {
    public:
        LineTestRenderer();
        ~LineTestRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        void clearData() override;

        //void updateData(const std::vector<PolylineData>& vPolylineDatas);
        void addPolyline(long long id, const float* verts, size_t n, float r, float g, float b);

    private:
        void uploadDataToGPU();  // 将所有折线数据上传到GPU
        struct PolylineData
        {
            long long id;
            std::vector<float> vertices;  // xyz坐标数组
            float r, g, b;                // 颜色
        };

        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;
        GLuint m_nEbo = 0;

        std::map<long long, PolylineData> m_polylines;  // 存储所有折线数据
        bool m_bDataDirty = true;  // 标记数据是否需要重新上传到GPU

        // Uniform
        int m_uCameraMatLoc = -1;
        int m_uColorLoc = -1;
        int m_uDepthLoc = -1;
    };
}
#endif // LINE_TEST_RENDERER_H