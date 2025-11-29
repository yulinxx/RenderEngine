#include "Render/RenderDataManager.h"
#include "FakeData/FakeDataProvider.h"
#include "FakeData/FakePolyLineData.h"
#include "DataManager/PolylinesVboManager.h"

#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <QDebug>

namespace GLRhi
{
    // Impl类定义，用于存储所有渲染数据
    class Impl
    {
    public:
        std::vector<PolylineData> m_lines;              // 存储线段数据
        std::vector<TriangleData> m_triangles;          // 存储三角形数据
        std::vector<TextureData> m_textures;            // 存储纹理数据
        std::vector<PolylineData> m_instanceLines;      // 存储实例化线段数据
        std::vector<TriangleData> m_instanceTriangles;  // 存储实例化三角形数据
        std::vector<TextureData> m_instanceTextures;    // 存储实例化纹理数据
    };

    // 构造函数
    RenderDataManager::RenderDataManager()
        : m_impl(new Impl())
    {}

    // 析构函数
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

        // 1. 随机删除20%的图元
        size_t removeCount = static_cast<size_t>(m_vPolylineDatas.size() * 0.2f);
        if (removeCount > 0)
        {
            // 随机打乱索引
            std::vector<size_t> indices(m_vPolylineDatas.size());
            for (size_t i = 0; i < m_vPolylineDatas.size(); ++i)
                indices[i] = i;
            
            std::shuffle(indices.begin(), indices.end(), std::default_random_engine(std::rand()));
            
            // 保留不需要删除的索引
            std::vector<PolylineData> newPolylineDatas;
            newPolylineDatas.reserve(m_vPolylineDatas.size() - removeCount);
            
            for (size_t i = removeCount; i < indices.size(); ++i)
                newPolylineDatas.push_back(m_vPolylineDatas[indices[i]]);
            
            m_vPolylineDatas = std::move(newPolylineDatas);
        }

        // 2. 修改15%的图元的顶点及颜色数据
        size_t modifyCount = static_cast<size_t>(m_vPolylineDatas.size() * 0.15f);
        if (modifyCount > 0)
        {
            // 随机选择要修改的索引
            std::vector<size_t> indices(m_vPolylineDatas.size());
            for (size_t i = 0; i < m_vPolylineDatas.size(); ++i)
                indices[i] = i;
            
            std::shuffle(indices.begin(), indices.end(), std::default_random_engine(std::rand()));
            
            // 修改选中的图元
            for (size_t i = 0; i < modifyCount && i < indices.size(); ++i)
            {
                size_t idx = indices[i];
                
                // 修改顶点数据 - 随机偏移
                for (size_t j = 0; j < m_vPolylineDatas[idx].vVerts.size(); j += 3)
                {
                    // 只修改x和y坐标，保持长度不变
                    float offsetX = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
                    float offsetY = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
                    m_vPolylineDatas[idx].vVerts[j] += offsetX;
                    m_vPolylineDatas[idx].vVerts[j + 1] += offsetY;
                }
                
                // 修改颜色数据
                float r = static_cast<float>(std::rand()) / RAND_MAX;
                float g = static_cast<float>(std::rand()) / RAND_MAX;
                float b = static_cast<float>(std::rand()) / RAND_MAX;

                m_vPolylineDatas[idx].brush.setRgb(r, g, b);
            }
        }

        // 3. 添加新数据 - 创建不固定数量的新线段，但确保总数不超过五百万
        
        // 设置最大值
        const size_t MAX_TOTAL_COUNT = 5000000;
        
        // 计算还可以添加的最大数量（随机添加1到10条，但不超过最大值限制）
        size_t currentCount = m_vPolylineDatas.size();
        size_t availableSlots = MAX_TOTAL_COUNT > currentCount ? MAX_TOTAL_COUNT - currentCount : 0;
        
        if (availableSlots > 0)
        {
            FakeDataProvider fakeDataProvider;
            std::vector<PolylineData> vPLineDatas = fakeDataProvider.genLineData(60);
            m_vPolylineDatas.insert(m_vPolylineDatas.end(), vPLineDatas.begin(), vPLineDatas.end());

        }
        else
        {
            qDebug() << "已达到最大线段数量限制 (" << MAX_TOTAL_COUNT << ")，无法添加新线段";
        }
    }

    void RenderDataManager::setTriangleDatas(std::vector<TriangleData> &datas)
    {
        m_vTriangleDatas = std::move(datas);
    }

    void RenderDataManager::setTextureDatas(std::vector<TextureData> &datas)
    {
        m_vTextureDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceTextureDatas(std::vector<InstanceTexData> &datas)
    {
        m_vInstanceTextureDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceLineDatas(std::vector<InstanceLineData> &datas)
    {
        m_vInstanceLineDatas = std::move(datas);
    }

    void RenderDataManager::setInstanceTriangleDatas(std::vector<InstanceTriangleData> &datas)
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
        // 查找并修改指定的线段
        // 注意：这里使用了简单的比较逻辑，实际应用中可能需要更复杂的比较
        for (auto& existingLine : m_impl->m_lines)
        {
            // 假设通过顶点数据和颜色进行简单比较
            // 在实际应用中，应该有更精确的标识方式
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
        // 查找并删除指定的线段
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

    // Triangle相关方法实现
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

    // Texture相关方法实现
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

    // Instance Line相关方法实现
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

    // Instance Triangle相关方法实现
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

    // Instance Texture相关方法实现
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