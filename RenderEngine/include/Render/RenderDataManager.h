#ifndef RENDER_DATA_MANAGER_H
#define RENDER_DATA_MANAGER_H

#include "Render/RenderCommon.h"
#include "Common/DllSet.h"

namespace GLRhi
{
    class Impl;

    class GLRENDER_API RenderDataManager final
    {
    public:
        RenderDataManager();
        ~RenderDataManager();

    public:
        void setPolylineDatas(std::vector<PolylineData>& datas);
        void setLineDatasCRUD();

        void setTriangleDatas(std::vector<TriangleData>& datas);
        void setTextureDatas(std::vector<TextureData>& datas);
        void setInstanceTextureDatas(std::vector<InstanceTexData>& datas);
        void setInstanceLineDatas(std::vector<InstanceLineData>& datas);
        void setInstanceTriangleDatas(std::vector<InstanceTriangleData>& datas);

        void addLine(const PolylineData& line);
        void addLines(const std::vector<PolylineData>& lines);

        void addTriangle(const TriangleData& triangle);
        void addTriangles(const std::vector<TriangleData>& triangles);

        void addTexture(const TextureData& texture);
        void addTextures(const std::vector<TextureData>& textures);

        void addInstanceLine(const PolylineData& instanceLine);
        void addInstanceLines(const std::vector<PolylineData>& instanceLines);

        void addInstanceTriangle(const TriangleData& instanceTriangle);
        void addInstanceTriangles(const std::vector<TriangleData>& instanceTriangles);

        void addInstanceTexture(const TextureData& instanceTexture);
        void addInstanceTextures(const std::vector<TextureData>& instanceTextures);

        /////////////////////
        void modifyLine(const PolylineData& line);
        void modifyLines(const std::vector<PolylineData>& lines);
        void modifyTriangle(const TriangleData& triangle);
        void modifyTriangles(const std::vector<TriangleData>& triangles);
        void modifyTexture(const TextureData& texture);

        void modifyInstanceLine(const PolylineData& instanceLine);
        void modifyInstanceLines(const std::vector<PolylineData>& instanceLines);
        void modifyInstanceTriangle(const TriangleData& instanceTriangle);
        void modifyInstanceTriangles(const std::vector<TriangleData>& instanceTriangles);
        void modifyInstanceTexture(const TextureData& instanceTexture);
        void modifyInstanceTextures(const std::vector<TextureData>& instanceTextures);

        /////////////////////
        void removeLine(const PolylineData& line);
        void removeLines(const std::vector<PolylineData>& lines);
        void removeTriangle(const TriangleData& triangle);
        void removeTriangles(const std::vector<TriangleData>& triangles);
        void removeTexture(const TextureData& texture);
        void removeTextures(const std::vector<TextureData>& textures);

        /////////////////////
        void removeInstanceLine(const PolylineData& instanceLine);
        void removeInstanceLines(const std::vector<PolylineData>& instanceLines);
        void removeInstanceTriangle(const TriangleData& instanceTriangle);
        void removeInstanceTriangles(const std::vector<TriangleData>& instanceTriangles);
        void removeInstanceTexture(const TextureData& instanceTexture);
        void removeInstanceTextures(const std::vector<TextureData>& instanceTextures);

        /////////////////////
        void deleteAll();

    private:
        Impl* m_impl{ nullptr };

        std::vector<PolylineData> m_vPolylineDatas; // 线段数据
        std::vector<TriangleData> m_vTriangleDatas; // 三角形数据
        std::vector<TextureData> m_vTextureDatas; // 纹理数据
        std::vector<InstanceTexData> m_vInstanceTextureDatas; // 实例纹理数据
        std::vector<InstanceLineData> m_vInstanceLineDatas; // 实例线段数据
        std::vector<InstanceTriangleData> m_vInstanceTriangleDatas; // 实例三角形数据
    };
}
#endif