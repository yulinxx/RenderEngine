#ifndef FAKE_TRIANGLE_H
#define FAKE_TRIANGLE_H

#include <vector>
#include "FakeDataBase.h"

class FakeTriangle final : public FakeDataBase
{
public:
    FakeTriangle();
    ~FakeTriangle() override;

public:
    // 生成指定数量的多边形并进行三角剖分
    // polygonCount: 生成的多边形数量
    // minVertices: 每个多边形的最小顶点数
    // maxVertices: 每个多边形的最大顶点数
    // concaveRatio: 凹多边形的比例（0.0-1.0），0.0表示全部凸多边形，1.0表示全部凹多边形
    void generatePolygons(size_t polygonCount = 1,
        size_t minVertices = 3,
        size_t maxVertices = 20,
        float concaveRatio = 0.5f);

    // 获取三角剖分后的顶点数据
    const std::vector<float>& getVertices() const;

    // 获取三角剖分后的索引数据
    const std::vector<uint32_t>& getIndices() const;

    // 获取多边形信息（每个多边形的原始顶点数）
    const std::vector<size_t>& getPolygonInfos() const;

    // 清空数据
    void clear() override;

private:
    // 生成单个凸多边形
    void generateConvexPolygon(int vertexCount);

    // 生成单个凹多边形
    void generateConcavePolygon(int vertexCount);

    // 对当前多边形进行三角剖分
    void triangulatePolygon(const std::vector<float>& polygonVertices);

private:
    std::vector<float> m_vertices;        // 三角剖分后的顶点数据，格式：x, y, z
    std::vector<uint32_t> m_indices;      // 三角剖分后的索引数据
    std::vector<size_t> m_polygonInfos;   // 每个多边形的原始顶点数
};

#endif // FAKE_TRIANGLE_H
