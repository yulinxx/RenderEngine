#ifndef FAKE_DATA_PROVIDER_H
#define FAKE_DATA_PROVIDER_H

#include <vector>
#include <QOpenGLFunctions_3_3_Core>

#include "DataManager/PolylinesVboManager.h"
#include "Render/RenderCommon.h"
#include "Common/PrimitiveIDGenerator.h"

/**
 * @brief 假数据提供者类
 * 用于生成各种测试和演示用的渲染数据
 */
class FakeDataProvider
{
public:
    FakeDataProvider();
    ~FakeDataProvider();

public:
    void initialize();

public:
    // 生成测试用线段数据
    std::vector<GLRhi::PolylineData> genLineData(
        size_t group = 20, size_t nLineSz = 100, size_t minPts = 2, size_t maxPts = 10);

    // 扰乱线段数据
    void disturbLineData(std::vector<GLRhi::PolylineData>& vPolylineDatas);
    void disturbLineDataVBO();

    // 生成测试用三角形数据
    std::vector<GLRhi::TriangleData> genTriangleData();
    // 生成测试用纹理数据
    std::vector<GLRhi::TextureData> genTextureData();
    // 生成测试用实例化纹理数据
    std::vector<GLRhi::InstanceTexData> genInstanceTextureData(GLuint& tex, int& vCount, size_t instanceCount = 20);

    // 清理资源
    void cleanup();

private:
    GLRhi::PrimitiveIDGenerator m_idGenerator;

    GLRhi::PolylinesVboManager m_plVboManager;

    std::vector<GLRhi::TriangleData> genBlendTestTriangleData();
    std::vector<GLRhi::TriangleData> genRandomTriangleData(size_t vCount = 10);
    std::vector<GLRhi::TextureData> genFileTextureData();
    std::vector<GLRhi::TextureData> genRandomTextureData(size_t vCount = 10);

    std::vector<GLRhi::PolylineData> m_vPolylineDatas;
};

#endif // DATA_GENERATOR_H