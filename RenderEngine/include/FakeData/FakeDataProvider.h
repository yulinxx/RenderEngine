#ifndef FAKE_DATA_PROVIDER_H
#define FAKE_DATA_PROVIDER_H

#include "Common/DllSet.h"

#include <vector>
#include <QOpenGLFunctions_3_3_Core>

#include "DataManager/PolylinesVboManager.h"
#include "Render/RenderCommon.h"
#include "Common/PrimitiveIDGenerator.h"


namespace GLRhi
{
    /**
     * @brief 假数据提供者类
     * 用于生成各种测试和演示用的渲染数据
     */
    class GLRENDER_API FakeDataProvider
    {
    public:
        FakeDataProvider();
        ~FakeDataProvider();

    public:
        void initialize();

    public:
        // 生成测试用线段数据
        std::vector<PolylineData> genLineData(
            size_t group =20, size_t nLineSz =100, size_t minPts = 2, size_t maxPts = 10);

        // 扰乱线段数据
        void disturbLineData(std::vector<PolylineData>& vPolylineDatas);
        void disturbLineDataVBO();

        // 生成测试用三角形数据
        std::vector<TriangleData> genTriangleData();
        // 生成测试用纹理数据
        std::vector<TextureData> genTextureData();
        // 生成测试用实例化纹理数据
        std::vector<InstanceTexData> genInstanceTextureData(GLuint &tex, int &vCount, size_t instanceCount =20);

        // 清理资源
        void cleanup();

    private:
        PrimitiveIDGenerator m_idGenerator;

        PolylinesVboManager m_plVboManager;

        std::vector<TriangleData> genBlendTestTriangleData();
        std::vector<TriangleData> genRandomTriangleData(size_t vCount = 10);
        std::vector<TextureData> genFileTextureData();
        std::vector<TextureData> genRandomTextureData(size_t vCount = 10);

        std::vector<PolylineData> m_vPolylineDatas;
    };
}

#endif // DATA_GENERATOR_H