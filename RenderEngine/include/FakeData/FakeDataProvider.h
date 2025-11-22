#ifndef FAKE_DATA_PROVIDER_H
#define FAKE_DATA_PROVIDER_H

#include <vector>
#include <QOpenGLFunctions_3_3_Core>

#include "Render/RenderCommon.h"
#include "Common/PrimitiveIDGenerator.h"
namespace GLRhi
{
    /**
     * @brief 假数据提供者类
     * 专门用于生成各种测试和演示用的渲染数据
     */
    class FakeDataProvider
    {
    public:
        FakeDataProvider();
        ~FakeDataProvider();

    public:
        void initialize(QOpenGLFunctions_3_3_Core *gl);

    public:
        // 生成测试用线段数据
        std::vector<PolylineData> genLineData(size_t count = 20);
        // 生成测试用三角形数据
        std::vector<TriangleData> genTriangleData();
        // 生成测试用纹理数据
        std::vector<TextureData> genTextureData();
        // 生成测试用实例化纹理数据
        std::vector<InstanceTexData> genInstanceTextureData(GLuint &tex, int &count);

        // 清理资源
        void cleanup();

    private:
        PrimitiveIDGenerator m_idGenerator;
        QOpenGLFunctions_3_3_Core *m_gl = nullptr;

        std::vector<TriangleData> genBlendTestTriangleData();
        std::vector<TriangleData> genRandomTriangleData();
        std::vector<TextureData> genFileTextureData();
        std::vector<TextureData> genRandomTextureData();
    };
}

#endif // DATA_GENERATOR_H