#ifndef FAKE_TRIANGLE_DATA_H
#define FAKE_TRIANGLE_DATA_H

#include <vector>
#include <random>

namespace GLRhi
{
    class FakeTriangleData
    {
    public:
        FakeTriangleData();
        ~FakeTriangleData();

    public:
        // 设置XY的最小/最大范围
        void setRange(float xMin, float xMax, float yMin, float yMax);

        // 生成指定数量的三角形
        // nTriangle: 要生成的三角形数量
        void generateTriangles(int nTriangle);

        // 获取生成的顶点数据（XYZ坐标 + RGB颜色）
        const std::vector<float>& getVertices() const;

        // 获取三角形索引数据
        const std::vector<unsigned int>& getIndices() const;

        // 清空数据
        void clear();

        // 生成随机浮点数
        float getRandomFloat(float min, float max);

    private:
        // 生成单个三角形
        void generateSingleTriangle();


    private:
        float m_xMin = -1.0f;           // X轴最小值
        float m_xMax = 1.0f;           // X轴最大值
        float m_yMin = -1.0f;           // Y轴最小值
        float m_yMax = 1.0f;           // Y轴最大值

        std::vector<float> m_vertices;     // 顶点数据，格式：x, y, depth
        std::vector<unsigned int> m_indices; // 三角形索引数据

        // 随机数生成器
        std::mt19937 m_generator;
        std::random_device m_randomDevice;
    };
}

#endif // FAKE_TRIANGLE_DATA_H