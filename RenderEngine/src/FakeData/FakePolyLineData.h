#ifndef FAKE_POLYLINE_DATA_H
#define FAKE_POLYLINE_DATA_H

#include <vector>
#include <random>

namespace GLRhi
{
    class FakePolyLineData
    {
    public:
        FakePolyLineData();
        ~FakePolyLineData();

    public:
        // 设置XY的最小/最大范围
        void setRange(float xMin, float xMax, float yMin, float yMax);

        // 生成指定数量的线段
        // lineCount:生成指定数量的线段
        // minPoints: 每条线段的最小点数
        // maxPoints: 每条线段的最大点数
        void generateLines(int lineCount = 1, int minPoints = 2, int maxPoints = 5000);

        // 获取生成的顶点数据
        const std::vector<float>& getVertices() const;

        // 获取线段信息（每条线段的点数）
        const std::vector<size_t>& getLineInfos() const;

        // 清空数据
        void clear();

        // 生成随机浮点数
        float getRandomFloat(float min, float max);

    private:
        // 生成单个线段的随机点
        void generateSingleLine(int pointCount);

    private:
        float m_xMin = -1000.0f;           // X轴最小值
        float m_xMax = 1000.0f;           // X轴最大值
        float m_yMin = -1000.0f;           // Y轴最小值
        float m_yMax = 1000.0f;           // Y轴最大值

        std::vector<float> m_vertices;                     // 顶点数据，格式：x, y, z (z始终为0)
        //std::vector<std::pair<size_t, size_t>> m_lineInfos; // 每条线段的起始索引和点数
        std::vector<size_t> m_lineInfos; // 每条线段的点数

        // 随机数生成器
        std::mt19937 m_generator;
        std::random_device m_randomDevice;
    };
}
#endif // FAKE_POLYLINE_DATA_H