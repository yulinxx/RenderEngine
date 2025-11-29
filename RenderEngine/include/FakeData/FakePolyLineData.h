#ifndef FAKE_POLYLINE_DATA_H
#define FAKE_POLYLINE_DATA_H

#include <vector>
#include "FakeDataBase.h"

namespace GLRhi
{
    class FakePolyLineData final : public FakeDataBase
    {
    public:
        FakePolyLineData();
        ~FakePolyLineData() override;

    public:
        // 生成指定数量的线段
        // lineCount:生成指定数量的线段
        // minPoints: 每条线段的最小点数
        // maxPoints: 每条线段的最大点数
        void generateLines(size_t lineCount = 1, size_t minPoints = 2, size_t maxPoints = 5000);

        // 获取生成的顶点数据
        const std::vector<float>& getVertices() const;

        // 获取线段信息（每条线段的点数）
        const std::vector<size_t>& getLineInfos() const;

        // 清空数据
        void clear() override;

        // 生成单个线段的随机点
        void generateSingleLine(int pointCount);

    private:
        std::vector<float> m_vertices;   // 顶点数据，格式：x, y, z
        std::vector<size_t> m_lineInfos; // 每条线段的点数
    };
}
#endif // FAKE_POLYLINE_DATA_H