#ifndef FAKE_TRIANGLE_DATA_H
#define FAKE_TRIANGLE_DATA_H

#include <vector>
#include "FakeData/FakeDataBase.h"

namespace GLRhi
{
    class FakeTriangleData : public FakeDataBase
    {
    public:
        FakeTriangleData();
        ~FakeTriangleData() override;

    public:
        // 生成指定数量的三角形
        // nTriangle: 要生成的三角形数量
        void generateTriangles(int nTriangle);

        // 获取生成的顶点数据（XYZ坐标 + RGB颜色）
        const std::vector<float>& getVertices() const;

        // 获取三角形索引数据
        const std::vector<unsigned int>& getIndices() const;

        // 清空数据
        void clear() override;

    private:
        // 生成单个三角形
        void generateSingleTriangle();

    private:
        std::vector<float> m_vertices;          // 顶点数据，格式：x, y, depth
        std::vector<unsigned int> m_indices;    // 三角形索引数据
    };
}

#endif // FAKE_TRIANGLE_DATA_H