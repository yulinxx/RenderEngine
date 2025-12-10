#ifndef FAKE_TRIANGLE_MESH_HELPER_H
#define FAKE_TRIANGLE_MESH_HELPER_H

#include "FakeTriangleMesh.h"
#include "Render/RenderCommon.h"
#include <vector>

// 辅助函数：将 FakeTriangleMesh 生成的数据转换为 GLRhi::TriangleData 格式
inline GLRhi::TriangleData convertToTriangleData(const FakeTriangleMesh& fakeTriangle,
    long long id = 0,
    const GLRhi::Brush& brush = GLRhi::Brush{ 1.0f, 0.5f, 0.0f, 1.0f, 0.0f })
{
    GLRhi::TriangleData triangleData;
    triangleData.id = id;
    triangleData.vVerts = fakeTriangle.getVertices();
    triangleData.vIndices = fakeTriangle.getIndices();
    triangleData.brush = brush;
    return triangleData;
}

// 辅助函数：批量转换多个 FakeTriangleMesh 数据
inline std::vector<GLRhi::TriangleData> convertToTriangleDatas(const std::vector<FakeTriangleMesh>& fakeTriangles,
    const std::vector<GLRhi::Brush>& brushes = {})
{
    std::vector<GLRhi::TriangleData> triangleDatas;
    triangleDatas.reserve(fakeTriangles.size());

    for (size_t i = 0; i < fakeTriangles.size(); ++i)
    {
        GLRhi::Brush brush = (i < brushes.size()) ? brushes[i] : GLRhi::Brush{ 1.0f, 0.5f, 0.0f, 1.0f, 0.0f };
        triangleDatas.push_back(convertToTriangleData(fakeTriangles[i], static_cast<long long>(i), brush));
    }

    return triangleDatas;
}

#endif // FAKE_TRIANGLE_MESH_HELPER_H
