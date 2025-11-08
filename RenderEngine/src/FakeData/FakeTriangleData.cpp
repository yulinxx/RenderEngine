#include "FakeTriangleData.h"

namespace GLRhi
{
    FakeTriangleData::FakeTriangleData()
    {
        try
        {
            m_generator = std::mt19937(m_randomDevice());
        }
        catch (...)
        {
            m_generator = std::mt19937(42);
        }
    }

    FakeTriangleData::~FakeTriangleData()
    {
        clear();
    }

    void FakeTriangleData::setRange(float xMin, float xMax, float yMin, float yMax)
    {
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
    }

    void FakeTriangleData::generateTriangles(int nTriangle)
    {
        if (nTriangle <= 0)
            return;

        // 清空现有数据
        clear();

        // 预分配内存以提高性能
        m_vertices.reserve(nTriangle * 3 * 3); // 每个三角形3个顶点，每个顶点3个浮点数(x,y,depth)
        m_indices.reserve(nTriangle * 3);     // 每个三角形3个索引

        // 生成指定数量的三角形
        for (int i = 0; i < nTriangle; ++i)
        {
            generateSingleTriangle();
        }
    }

    const std::vector<float>& FakeTriangleData::getVertices() const
    {
        return m_vertices;
    }

    const std::vector<unsigned int>& FakeTriangleData::getIndices() const
    {
        return m_indices;
    }

    void FakeTriangleData::clear()
    {
        m_vertices.clear();
        m_vertices.shrink_to_fit();

        m_indices.clear();
        m_indices.shrink_to_fit();
    }

    void FakeTriangleData::generateSingleTriangle()
    {
        // 三角形的大小范围
        float minTriangleSize = std::min((m_xMax - m_xMin), (m_yMax - m_yMin)) * 0.01f; // 最小为范围的1%
        float maxTriangleSize = std::min((m_xMax - m_xMin), (m_yMax - m_yMin)) * 0.05f; // 最大为范围的5%
        float triangleSize = getRandomFloat(minTriangleSize, maxTriangleSize);

        // 生成三角形的中心点
        float centerX = getRandomFloat(m_xMin + maxTriangleSize, m_xMax - maxTriangleSize);
        float centerY = getRandomFloat(m_yMin + maxTriangleSize, m_yMax - maxTriangleSize);
        float depth = 0.0f; // 固定深度为0

        // 计算三角形的三个顶点
        // 使用等边三角形，随机旋转
        float angleOffset = getRandomFloat(0.0f, 6.283185307f); // 0到2π的随机角度

        // 三个顶点的角度（相差120度）
        float angle1 = angleOffset;
        float angle2 = angleOffset + 2.094395102f; // 120度 = 2π/3
        float angle3 = angleOffset + 4.188790205f; // 240度 = 4π/3

        // 计算三个顶点的坐标
        float x1 = centerX + triangleSize * cos(angle1);
        float y1 = centerY + triangleSize * sin(angle1);

        float x2 = centerX + triangleSize * cos(angle2);
        float y2 = centerY + triangleSize * sin(angle2);

        float x3 = centerX + triangleSize * cos(angle3);
        float y3 = centerY + triangleSize * sin(angle3);

        // 添加三个顶点数据
        m_vertices.push_back(x1);
        m_vertices.push_back(y1);
        m_vertices.push_back(depth);

        m_vertices.push_back(x2);
        m_vertices.push_back(y2);
        m_vertices.push_back(depth);

        m_vertices.push_back(x3);
        m_vertices.push_back(y3);
        m_vertices.push_back(depth);

        // 添加索引
        // 计算基础索引：每个顶点3个浮点数，所以除以3
        unsigned int vertexCount = static_cast<unsigned int>(m_vertices.size() / 3);
        // 新添加的三个顶点的索引是 vertexCount - 3, vertexCount - 2, vertexCount - 1
        m_indices.push_back(vertexCount - 3);
        m_indices.push_back(vertexCount - 2);
        m_indices.push_back(vertexCount - 1);

        //// 添加三个顶点数据
        //m_vertices.insert(m_vertices.end(), {
        //    x1, y1, depth,
        //    x2, y2, depth,
        //    x3, y3, depth,
        //    });

        //// 添加索引
        //m_indices.insert(m_indices.end(), {
        //    baseIndex,
        //    baseIndex + 1,
        //    baseIndex + 2
        //    });
    }

    float FakeTriangleData::getRandomFloat(float min, float max)
    {
        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(m_generator);
    }
}