#include "FakeData/FakeTriangle.h"
#include "3rdpart/earcut.hpp"
#include <array>
#include <cmath>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FakeTriangle::FakeTriangle()
{
    // 可以设置默认范围
    // setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);
}

FakeTriangle::~FakeTriangle()
{
    clear();
}

void FakeTriangle::generatePolygons(size_t polygonCount, size_t minVertices, size_t maxVertices, float concaveRatio)
{
    if (minVertices < 3)
        minVertices = 3;
    if (maxVertices > 100)
        maxVertices = 100;
    if (minVertices > maxVertices)
        minVertices = maxVertices;
    if (concaveRatio < 0.0f)
        concaveRatio = 0.0f;
    if (concaveRatio > 1.0f)
        concaveRatio = 1.0f;

    clear();

    // 如果生成数量较少（<= 20），启用防重叠检测
    bool enableOverlapCheck = (polygonCount <= 20);

    // 生成指定数量的多边形
    for (size_t i = 0; i < polygonCount; ++i)
    {
        int vertexCount = getRandomInt(static_cast<int>(minVertices), static_cast<int>(maxVertices));
        
        // 如果启用防重叠，尝试多次生成不重叠的多边形
        int maxRetries = enableOverlapCheck ? 50 : 1;
        bool success = false;
        
        for (int retry = 0; retry < maxRetries; ++retry)
        {
            size_t vertexCountBefore = m_vertices.size();
            size_t indexCountBefore = m_indices.size();
            size_t bboxCountBefore = m_boundingBoxes.size();
            
            // 根据凹多边形比例决定生成凸多边形还是凹多边形
            float randValue = getRandomFloat(0.0f, 1.0f);
            if (randValue < concaveRatio)
            {
                generateConcavePolygon(vertexCount);
            }
            else
            {
                generateConvexPolygon(vertexCount);
            }
            
            // 如果不需要检查重叠，或者没有重叠，则成功
            if (!enableOverlapCheck || m_boundingBoxes.size() <= 1)
            {
                success = true;
                break;
            }
            
            // 检查新生成的多边形是否与已有多边形重叠
            const auto& newBBox = m_boundingBoxes.back();
            bool hasOverlap = false;
            
            for (size_t j = 0; j < m_boundingBoxes.size() - 1; ++j)
            {
                const auto& existingBBox = m_boundingBoxes[j];
                
                // AABB 碰撞检测
                if (!(newBBox.maxX < existingBBox.minX || 
                      newBBox.minX > existingBBox.maxX ||
                      newBBox.maxY < existingBBox.minY || 
                      newBBox.minY > existingBBox.maxY))
                {
                    hasOverlap = true;
                    break;
                }
            }
            
            if (!hasOverlap)
            {
                success = true;
                break;
            }
            
            // 有重叠，回退并重试
            m_vertices.resize(vertexCountBefore);
            m_indices.resize(indexCountBefore);
            m_boundingBoxes.resize(bboxCountBefore);
            
            if (m_polygonInfos.size() > 0)
                m_polygonInfos.pop_back();
        }
        
        if (!success && enableOverlapCheck)
        {
            qDebug() << "Warning: Failed to generate non-overlapping polygon after" << maxRetries << "retries";
        }
    }
}

const std::vector<float>& FakeTriangle::getVertices() const
{
    return m_vertices;
}

const std::vector<uint32_t>& FakeTriangle::getIndices() const
{
    return m_indices;
}

const std::vector<size_t>& FakeTriangle::getPolygonInfos() const
{
    return m_polygonInfos;
}

void FakeTriangle::clear()
{
    m_vertices.clear();
    m_indices.clear();
    m_polygonInfos.clear();
    m_boundingBoxes.clear();
}

void FakeTriangle::generateConvexPolygon(int vertexCount)
{
    if (vertexCount < 3)
        vertexCount = 3;

    // 生成凸多边形：使用极坐标方法，在圆周上随机分布点
    float centerX = getRandomFloat(m_xMin * 0.5f, m_xMax * 0.5f);
    float centerY = getRandomFloat(m_yMin * 0.5f, m_yMax * 0.5f);

    // 计算最大半径（确保不超出边界）
    float maxRadius = std::min({
        (m_xMax - m_xMin) * 0.2f,
        (m_yMax - m_yMin) * 0.2f,
        std::abs(m_xMax - centerX),
        std::abs(centerX - m_xMin),
        std::abs(m_yMax - centerY),
        std::abs(centerY - m_yMin)
        }) * 0.8f;

    // 生成随机角度并排序（保证凸多边形）
    std::vector<float> angles;
    for (int i = 0; i < vertexCount; ++i)
    {
        angles.push_back(getRandomFloat(0.0f, static_cast<float>(2.0 * M_PI)));
    }
    std::sort(angles.begin(), angles.end());

    // 为每个角度生成随机半径，创建顶点
    std::vector<float> polygonVertices;
    for (int i = 0; i < vertexCount; ++i)
    {
        float radius = getRandomFloat(maxRadius * 0.3f, maxRadius);
        float x = centerX + radius * std::cos(angles[i]);
        float y = centerY + radius * std::sin(angles[i]);
        float z = 0.0f;

        polygonVertices.push_back(x);
        polygonVertices.push_back(y);
        polygonVertices.push_back(z);
    }

    // 记录多边形信息
    m_polygonInfos.push_back(vertexCount);

    // 进行三角剖分
    triangulatePolygon(polygonVertices);
}

void FakeTriangle::generateConcavePolygon(int vertexCount)
{
    if (vertexCount < 3)
        vertexCount = 3;

    // 生成凹多边形：通过在凸多边形基础上添加一些"凹陷"
    float centerX = getRandomFloat(m_xMin * 0.5f, m_xMax * 0.5f);
    float centerY = getRandomFloat(m_yMin * 0.5f, m_yMax * 0.5f);

    float maxRadius = std::min({
        (m_xMax - m_xMin) * 0.2f,
        (m_yMax - m_yMin) * 0.2f,
        std::abs(m_xMax - centerX),
        std::abs(centerX - m_xMin),
        std::abs(m_yMax - centerY),
        std::abs(centerY - m_yMin)
        }) * 0.8f;

    // 生成随机角度并排序
    std::vector<float> angles;
    for (int i = 0; i < vertexCount; ++i)
    {
        angles.push_back(getRandomFloat(0.0f, static_cast<float>(2.0 * M_PI)));
    }
    std::sort(angles.begin(), angles.end());

    // 为每个角度生成随机半径，某些顶点使用较小半径制造凹陷
    std::vector<float> polygonVertices;
    for (int i = 0; i < vertexCount; ++i)
    {
        float radius;
        // 随机选择一些顶点使用较小半径（创造凹陷效果）
        if (getRandomFloat(0.0f, 1.0f) < 0.3f && vertexCount > 5) // 30%的概率创建凹陷
        {
            radius = getRandomFloat(maxRadius * 0.1f, maxRadius * 0.4f);
        }
        else
        {
            radius = getRandomFloat(maxRadius * 0.5f, maxRadius);
        }

        float x = centerX + radius * std::cos(angles[i]);
        float y = centerY + radius * std::sin(angles[i]);
        float z = 0.0f;

        polygonVertices.push_back(x);
        polygonVertices.push_back(y);
        polygonVertices.push_back(z);
    }

    // 记录多边形信息
    m_polygonInfos.push_back(vertexCount);

    // 进行三角剖分
    triangulatePolygon(polygonVertices);
}

void FakeTriangle::triangulatePolygon(const std::vector<float>& polygonVertices)
{
    if (polygonVertices.size() < 9) // 至少需要3个顶点（每个顶点3个坐标）
        return;

    // 准备earcut所需的数据结构
    // earcut期望的输入格式：vector<vector<array<double, 2>>>
    using Point = std::array<double, 2>;
    std::vector<std::vector<Point>> polygon;
    std::vector<Point> outerRing;

    // 将顶点数据转换为earcut需要的格式（只使用x, y坐标）
    for (size_t i = 0; i < polygonVertices.size(); i += 3)
    {
        Point p = { static_cast<double>(polygonVertices[i]),
                   static_cast<double>(polygonVertices[i + 1]) };
        outerRing.push_back(p);
    }

    polygon.push_back(outerRing);

    // 执行三角剖分
    std::vector<uint32_t> triangleIndices = mapbox::earcut<uint32_t>(polygon);

    // 添加顶点数据（如果这是第一个多边形，直接添加；否则需要调整索引）
    uint32_t vertexOffset = static_cast<uint32_t>(m_vertices.size() / 3);

    // 添加顶点
    m_vertices.insert(m_vertices.end(), polygonVertices.begin(), polygonVertices.end());

    // 添加索引（需要加上偏移量）
    for (uint32_t index : triangleIndices)
    {
        m_indices.push_back(vertexOffset + index);
    }

        // 计算并保存边界框
        if (!polygonVertices.empty())
        {
            BoundingBox bbox;
            bbox.minX = bbox.maxX = polygonVertices[0];
            bbox.minY = bbox.maxY = polygonVertices[1];
            
            for (size_t i = 0; i < polygonVertices.size(); i += 3)
            {
                float x = polygonVertices[i];
                float y = polygonVertices[i + 1];
                
                bbox.minX = std::min(bbox.minX, x);
                bbox.maxX = std::max(bbox.maxX, x);
                bbox.minY = std::min(bbox.minY, y);
                bbox.maxY = std::max(bbox.maxY, y);
            }
            
            m_boundingBoxes.push_back(bbox);
        }
        
        // 调试信息
        qDebug() << "Generated polygon with" << (polygonVertices.size() / 3) 
                 << "vertices, triangulated into" << (triangleIndices.size() / 3) << "triangles";
    }