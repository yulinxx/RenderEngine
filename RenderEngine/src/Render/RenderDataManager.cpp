#include "Render/RenderDataManager.h"
#include <vector>
#include <QDebug>

#include "FakeData/FakeDataProvider.h"

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
        // 1. 删除部分数据
        std::vector<PolylineData> vLinesToRemove;
        for (const auto& line : m_vPolylineDatas)
        {
           if (line.id % 20 == 1)
           {
               vLinesToRemove.push_back(line);
           }
        }

        if (!vLinesToRemove.empty())
        {
           qDebug() << "Removed" << vLinesToRemove.size() << "line segments";
        }

        // 2. 修改部分数据
        std::vector<PolylineData> linesToModify;
        for (auto& line : m_vPolylineDatas)
        {
           if (line.id % 2 == 0)
           {
               line.brush = { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };
               linesToModify.push_back(line);
           }
        }
        if (!linesToModify.empty())
        {
           modifyLines(linesToModify);
           qDebug() << "修改了" << linesToModify.size() << "条偶数ID的线段数据，设置为蓝色";
        }

        // 3. 添加新数据 - 创建2条新的线段
        std::vector<PolylineData> linesToAdd;
        
        // 创建第一条新线段
        PolylineData newLine1;
        newLine1.id = -1; // 让系统自动分配ID或使用特殊ID标记
        newLine1.verts = { -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f }; // 从(-0.5,-0.5)到(0.5,0.5)的线段
        newLine1.count = { 2 }; // 2个顶点
        newLine1.brush = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f }; // 红色
        linesToAdd.push_back(newLine1);
        
        // 创建第二条新线段
        PolylineData newLine2;
        newLine2.id = -2; // 让系统自动分配ID或使用特殊ID标记
        newLine2.verts = { 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f }; // 从(0.5,-0.5)到(-0.5,0.5)的线段
        newLine2.count = { 2 }; // 2个顶点
        newLine2.brush = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f }; // 绿色
        linesToAdd.push_back(newLine2);
        
        if (!linesToAdd.empty())
        {
           addLines(linesToAdd);
           qDebug() << "添加了" << linesToAdd.size() << "条新的线段数据";
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
            if (existingLine.verts == line.verts && 
                existingLine.count == line.count && 
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
            if (it->verts == line.verts && 
                it->count == line.count && 
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
            if (existingTriangle.verts == triangle.verts && 
                existingTriangle.indices == triangle.indices && 
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
            if (it->verts == triangle.verts && 
                it->indices == triangle.indices && 
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
            if (existingTexture.verts == texture.verts && 
                existingTexture.indices == texture.indices && 
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
            if (it->verts == texture.verts && 
                it->indices == texture.indices && 
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
            if (existingLine.verts == instanceLine.verts && 
                existingLine.count == instanceLine.count && 
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
            if (it->verts == instanceLine.verts && 
                it->count == instanceLine.count && 
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
            if (existingTriangle.verts == instanceTriangle.verts && 
                existingTriangle.indices == instanceTriangle.indices && 
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
            if (it->verts == instanceTriangle.verts && 
                it->indices == instanceTriangle.indices && 
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
            if (existingTexture.verts == instanceTexture.verts && 
                existingTexture.indices == instanceTexture.indices && 
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
            if (it->verts == instanceTexture.verts && 
                it->indices == instanceTexture.indices && 
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