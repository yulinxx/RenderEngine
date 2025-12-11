#include "Render/RenderDataManager.h"
#include "DataManager/PolylinesVboManager.h"

#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <QDebug>

namespace GLRhi
{
    class Impl
    {
    public:
        std::vector<PolylineData> m_lines;              // 线段
        std::vector<TriangleData> m_triangles;          // 三角形
        std::vector<TextureData> m_textures;            // 纹理
        std::vector<PolylineData> m_instanceLines;      // 实例化线段
        std::vector<TriangleData> m_instanceTriangles;  // 实例化三角形
        std::vector<TextureData> m_instanceTextures;    // 实例化纹理
    };

    RenderDataManager::RenderDataManager()
        : m_impl(new Impl())
    {
    }

    RenderDataManager::~RenderDataManager()
    {
        delete m_impl;
    }

    void RenderDataManager::setPolylineDatas(std::vector<PolylineData>& datas)
    {
        m_vPolylineDatas = std::move(datas);
    }

    void RenderDataManager::setLineDatasCRUD()
    {
        if (m_vPolylineDatas.empty())
            return;

        // 随机删除图元
        size_t removeCount = static_cast<size_t>(m_vPolylineDatas.size() * 0.2f);
        if (removeCount > 0)
        {
            std::vector<size_t> indices(m_vPolylineDatas.size());
            for (size_t i = 0; i < m_vPolylineDatas.size(); ++i)
                indices[i] = i;

            std::shuffle(indices.begin(), indices.end(), std::default_random_engine(std::rand()));

            std::vector<PolylineData> newPolylineDatas;
            newPolylineDatas.reserve(m_vPolylineDatas.size() - removeCount);

            for (size_t i = removeCount; i < indices.size(); ++i)
                newPolylineDatas.push_back(m_vPolylineDatas[indices[i]]);

            m_vPolylineDatas = std::move(newPolylineDatas);
        }

        // 修改图元数据
        size_t modifyCount = static_cast<size_t>(m_vPolylineDatas.size() * 0.15f);
        if (modifyCount > 0)
        {
            std::vector<size_t> indices(m_vPolylineDatas.size());
            for (size_t i = 0; i < m_vPolylineDatas.size(); ++i)
                indices[i] = i;

            std::shuffle(indices.begin(), indices.end(), std::default_random_engine(std::rand()));

            for (size_t i = 0; i < modifyCount && i < indices.size(); ++i)
            {
                size_t idx = indices[i];

                for (size_t j = 0; j < m_vPolylineDatas[idx].vVerts.size(); j += 3)
                {
                    float offsetX = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
                    float offsetY = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
                    m_vPolylineDatas[idx].vVerts[j] += offsetX;
                    m_vPolylineDatas[idx].vVerts[j + 1] += offsetY;
                }

                float r = static_cast<float>(std::rand()) / RAND_MAX;
                float g = static_cast<float>(std::rand()) / RAND_MAX;
                float b = static_cast<float>(std::rand()) / RAND_MAX;

                m_vPolylineDatas[idx].brush.setRgb(r, g, b);
            }
        }

        // 添加新数据

        const size_t MAX_COUNT = 5000000;

        size_t currentCount = m_vPolylineDatas.size();
        size_t availableSlots = MAX_COUNT > currentCount ? MAX_COUNT - currentCount : 0;

        if (availableSlots > 0)
        {
            // 简单的随机线段生成，替代FakeDataProvider
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
            std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
            std::uniform_int_distribution<int> pointCountDist(2, 10);
            std::uniform_real_distribution<float> lineWidthDist(1.0f, 5.0f); // 随机线宽分布

            // 生成少量随机线段
            size_t newLineCount = std::min(availableSlots, static_cast<size_t>(10));
            for (size_t i = 0; i < newLineCount; ++i)
            {
                PolylineData line;

                // 随机生成线段的点
                int pointCount = pointCountDist(gen);
                for (int j = 0; j < pointCount; ++j)
                {
                    line.vVerts.push_back(posDist(gen)); // x
                    line.vVerts.push_back(posDist(gen)); // y
                    line.vVerts.push_back(0.0f);          // z
                }

                // 随机颜色
                line.brush.set(colorDist(gen), colorDist(gen), colorDist(gen), 1.0f);

                // 线宽已经在PolylinesVboManager中处理，不再需要在这里设置

                // 添加到数据中
                m_vPolylineDatas.push_back(line);
            }
        }
        else
        {
            qDebug() << "已达到最大线段数量限制 (" << MAX_COUNT << ")";
        }
    }

    void RenderDataManager::setTriangleDatas(std::vector<TriangleData>& datas)
    {
        m_vTriangleDatas = std::move(datas);
    }

    void RenderDataManager::setTextureDatas(std::vector<TextureData>& datas)
    {
        m_vTextureDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceTextureDatas(std::vector<InstanceTexData>& datas)
    {
        m_vInstanceTextureDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceLineDatas(std::vector<InstanceLineData>& datas)
    {
        m_vInstanceLineDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceTriangleDatas(std::vector<InstanceTriangleData>& datas)
    {
        m_vInstanceTriangleDatas = std::move(datas);
    }

    void RenderDataManager::addLine(const PolylineData& line)
    {
        m_impl->m_lines.push_back(line);
    }

    void RenderDataManager::addLines(const std::vector<PolylineData>& lines)
    {
        m_impl->m_lines.insert(m_impl->m_lines.end(), lines.begin(), lines.end());
    }

    void RenderDataManager::modifyLine(const PolylineData& line)
    {
        for (auto& existingLine : m_impl->m_lines)
        {
            if (existingLine.vVerts == line.vVerts &&
                existingLine.vCount == line.vCount &&
                existingLine.brush == line.brush)
            {
                existingLine = line;
                break;
            }
        }
    }

    void RenderDataManager::modifyLines(const std::vector<PolylineData>& lines)
    {
        for (const auto& line : lines)
        {
            modifyLine(line);
        }
    }

    void RenderDataManager::removeLine(const PolylineData& line)
    {
        for (auto it = m_impl->m_lines.begin(); it != m_impl->m_lines.end(); ++it)
        {
            if (it->vVerts == line.vVerts &&
                it->vCount == line.vCount &&
                it->brush == line.brush)
            {
                m_impl->m_lines.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeLines(const std::vector<PolylineData>& lines)
    {
        for (const auto& line : lines)
        {
            removeLine(line);
        }
    }

    void RenderDataManager::addTriangle(const TriangleData& triangle)
    {
        m_impl->m_triangles.push_back(triangle);
    }

    void RenderDataManager::addTriangles(const std::vector<TriangleData>& triangles)
    {
        m_impl->m_triangles.insert(m_impl->m_triangles.end(), triangles.begin(), triangles.end());
    }

    void RenderDataManager::modifyTriangle(const TriangleData& triangle)
    {
        for (auto& existingTriangle : m_impl->m_triangles)
        {
            if (existingTriangle.vVerts == triangle.vVerts &&
                existingTriangle.vIndices == triangle.vIndices &&
                existingTriangle.brush == triangle.brush)
            {
                existingTriangle = triangle;
                break;
            }
        }
    }

    void RenderDataManager::modifyTriangles(const std::vector<TriangleData>& triangles)
    {
        for (const auto& triangle : triangles)
        {
            modifyTriangle(triangle);
        }
    }

    void RenderDataManager::removeTriangle(const TriangleData& triangle)
    {
        for (auto it = m_impl->m_triangles.begin(); it != m_impl->m_triangles.end(); ++it)
        {
            if (it->vVerts == triangle.vVerts &&
                it->vIndices == triangle.vIndices &&
                it->brush == triangle.brush)
            {
                m_impl->m_triangles.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeTriangles(const std::vector<TriangleData>& triangles)
    {
        for (const auto& triangle : triangles)
        {
            removeTriangle(triangle);
        }
    }

    void RenderDataManager::addTexture(const TextureData& texture)
    {
        m_impl->m_textures.push_back(texture);
    }

    void RenderDataManager::addTextures(const std::vector<TextureData>& textures)
    {
        m_impl->m_textures.insert(m_impl->m_textures.end(), textures.begin(), textures.end());
    }

    void RenderDataManager::modifyTexture(const TextureData& texture)
    {
        for (auto& existingTexture : m_impl->m_textures)
        {
            if (existingTexture.vVerts == texture.vVerts &&
                existingTexture.vIndices == texture.vIndices &&
                existingTexture.tex == texture.tex &&
                existingTexture.brush == texture.brush)
            {
                existingTexture = texture;
                break;
            }
        }
    }

    void RenderDataManager::removeTexture(const TextureData& texture)
    {
        for (auto it = m_impl->m_textures.begin(); it != m_impl->m_textures.end(); ++it)
        {
            if (it->vVerts == texture.vVerts &&
                it->vIndices == texture.vIndices &&
                it->tex == texture.tex &&
                it->brush == texture.brush)
            {
                m_impl->m_textures.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeTextures(const std::vector<TextureData>& textures)
    {
        for (const auto& texture : textures)
        {
            removeTexture(texture);
        }
    }

    void RenderDataManager::addInstanceLine(const PolylineData& instanceLine)
    {
        m_impl->m_instanceLines.push_back(instanceLine);
    }

    void RenderDataManager::addInstanceLines(const std::vector<PolylineData>& instanceLines)
    {
        m_impl->m_instanceLines.insert(m_impl->m_instanceLines.end(), instanceLines.begin(), instanceLines.end());
    }

    void RenderDataManager::modifyInstanceLine(const PolylineData& instanceLine)
    {
        for (auto& existingLine : m_impl->m_instanceLines)
        {
            if (existingLine.vVerts == instanceLine.vVerts &&
                existingLine.vCount == instanceLine.vCount &&
                existingLine.brush == instanceLine.brush)
            {
                existingLine = instanceLine;
                break;
            }
        }
    }

    void RenderDataManager::modifyInstanceLines(const std::vector<PolylineData>& instanceLines)
    {
        for (const auto& line : instanceLines)
        {
            modifyInstanceLine(line);
        }
    }

    void RenderDataManager::removeInstanceLine(const PolylineData& instanceLine)
    {
        for (auto it = m_impl->m_instanceLines.begin(); it != m_impl->m_instanceLines.end(); ++it)
        {
            if (it->vVerts == instanceLine.vVerts &&
                it->vCount == instanceLine.vCount &&
                it->brush == instanceLine.brush)
            {
                m_impl->m_instanceLines.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeInstanceLines(const std::vector<PolylineData>& instanceLines)
    {
        for (const auto& line : instanceLines)
        {
            removeInstanceLine(line);
        }
    }

    void RenderDataManager::addInstanceTriangle(const TriangleData& instanceTriangle)
    {
        m_impl->m_instanceTriangles.push_back(instanceTriangle);
    }

    void RenderDataManager::addInstanceTriangles(const std::vector<TriangleData>& instanceTriangles)
    {
        m_impl->m_instanceTriangles.insert(m_impl->m_instanceTriangles.end(), instanceTriangles.begin(), instanceTriangles.end());
    }

    void RenderDataManager::modifyInstanceTriangle(const TriangleData& instanceTriangle)
    {
        for (auto& existingTriangle : m_impl->m_instanceTriangles)
        {
            if (existingTriangle.vVerts == instanceTriangle.vVerts &&
                existingTriangle.vIndices == instanceTriangle.vIndices &&
                existingTriangle.brush == instanceTriangle.brush)
            {
                existingTriangle = instanceTriangle;
                break;
            }
        }
    }

    void RenderDataManager::modifyInstanceTriangles(const std::vector<TriangleData>& instanceTriangles)
    {
        for (const auto& triangle : instanceTriangles)
        {
            modifyInstanceTriangle(triangle);
        }
    }

    void RenderDataManager::removeInstanceTriangle(const TriangleData& instanceTriangle)
    {
        for (auto it = m_impl->m_instanceTriangles.begin(); it != m_impl->m_instanceTriangles.end(); ++it)
        {
            if (it->vVerts == instanceTriangle.vVerts &&
                it->vIndices == instanceTriangle.vIndices &&
                it->brush == instanceTriangle.brush)
            {
                m_impl->m_instanceTriangles.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeInstanceTriangles(const std::vector<TriangleData>& instanceTriangles)
    {
        for (const auto& triangle : instanceTriangles)
        {
            removeInstanceTriangle(triangle);
        }
    }

    void RenderDataManager::addInstanceTexture(const TextureData& instanceTexture)
    {
        m_impl->m_instanceTextures.push_back(instanceTexture);
    }

    void RenderDataManager::addInstanceTextures(const std::vector<TextureData>& instanceTextures)
    {
        m_impl->m_instanceTextures.insert(m_impl->m_instanceTextures.end(), instanceTextures.begin(), instanceTextures.end());
    }

    void RenderDataManager::modifyInstanceTexture(const TextureData& instanceTexture)
    {
        for (auto& existingTexture : m_impl->m_instanceTextures)
        {
            if (existingTexture.vVerts == instanceTexture.vVerts &&
                existingTexture.vIndices == instanceTexture.vIndices &&
                existingTexture.tex == instanceTexture.tex &&
                existingTexture.brush == instanceTexture.brush)
            {
                existingTexture = instanceTexture;
                break;
            }
        }
    }

    void RenderDataManager::modifyInstanceTextures(const std::vector<TextureData>& instanceTextures)
    {
        for (const auto& texture : instanceTextures)
        {
            modifyInstanceTexture(texture);
        }
    }

    void RenderDataManager::removeInstanceTexture(const TextureData& instanceTexture)
    {
        for (auto it = m_impl->m_instanceTextures.begin(); it != m_impl->m_instanceTextures.end(); ++it)
        {
            if (it->vVerts == instanceTexture.vVerts &&
                it->vIndices == instanceTexture.vIndices &&
                it->tex == instanceTexture.tex &&
                it->brush == instanceTexture.brush)
            {
                m_impl->m_instanceTextures.erase(it);
                break;
            }
        }
    }

    void RenderDataManager::removeInstanceTextures(const std::vector<TextureData>& instanceTextures)
    {
        for (const auto& texture : instanceTextures)
        {
            removeInstanceTexture(texture);
        }
    }

    // 清理所有数据
    void RenderDataManager::deleteAll()
    {
        m_impl->m_lines.clear();
        m_impl->m_triangles.clear();
        m_impl->m_textures.clear();
        m_impl->m_instanceLines.clear();
        m_impl->m_instanceTriangles.clear();
        m_impl->m_instanceTextures.clear();
    }
}