#include "FakeData/InstanceTriangleFakeData.h"

using namespace GLRhi;

InstanceTriangleFakeData::InstanceTriangleFakeData()
{
}

InstanceTriangleFakeData::~InstanceTriangleFakeData()
{
    clear();
}

void InstanceTriangleFakeData::genTriangles(int triangleCount, float minSize, float maxSize)
{
    clear();
    m_instanceData.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i)
    {
        InstanceTriangleData triangleData = genSingleTriangle(minSize, maxSize);
        m_instanceData.push_back(triangleData);
    }
}

const std::vector<InstanceTriangleData>& InstanceTriangleFakeData::getInstanceData() const
{
    return m_instanceData;
}

void InstanceTriangleFakeData::clear()
{
    m_instanceData.clear();
}

InstanceTriangleData InstanceTriangleFakeData::genSingleTriangle(float minSize, float maxSize)
{
    InstanceTriangleData triangleData;

    // 生成三角形中心点
    float centerX = getRandomFloat(m_xMin, m_xMax);
    float centerY = getRandomFloat(m_yMin, m_yMax);
    float size = getRandomFloat(minSize, maxSize);

    // 计算三角形的三个顶点
    // 顶点1：中心点正上方
    triangleData.pos1[0] = centerX;
    triangleData.pos1[1] = centerY + size;
    triangleData.pos1[2] = 0.0f;

    // 顶点2：中心点左下方
    triangleData.pos2[0] = centerX - size * 0.866f; // cos(240°) = -√3/2 ≈ -0.866
    triangleData.pos2[1] = centerY - size * 0.5f;   // sin(240°) = -1/2
    triangleData.pos2[2] = 0.0f;

    // 顶点3：中心点右下方
    triangleData.pos3[0] = centerX + size * 0.866f; // cos(60°) = √3/2 ≈ 0.866
    triangleData.pos3[1] = centerY - size * 0.5f;   // sin(60°) = -1/2
    triangleData.pos3[2] = 0.0f;

    // 生成随机颜色
    Color color = FakeDataBase::genRandomColor();
    triangleData.color[0] = color.getRed();
    triangleData.color[1] = color.getGreen();
    triangleData.color[2] = color.getBlue();
    triangleData.color[3] = color.getAlpha();

    // 生成随机深度值（用于绘制顺序）
    triangleData.depth = getRandomFloat(0.0f, 1.0f);

    return triangleData;
}