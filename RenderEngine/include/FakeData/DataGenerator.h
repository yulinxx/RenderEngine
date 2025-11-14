#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <QOpenGLFunctions_3_3_Core>

#include "Render/RenderCommon.h"

namespace GLRhi
{
    /**
     * @brief 数据生成器类
     */
    class DataGenerator
    {
    public:
        DataGenerator();
        ~DataGenerator();

    public:
        void initialize(QOpenGLFunctions_3_3_Core* gl);

        std::vector<PolylineData> genLineData();
        std::vector<TriangleData> genTriangleData();
        std::vector<TextureData> genTextureData();
        std::vector<InstanceTexData> genInstanceTextureData(GLuint& tex, int& count);

        void cleanup();

    private:
        QOpenGLFunctions_3_3_Core* m_gl = nullptr;

        std::vector<TriangleData> genBlendTestTriangleData();
        std::vector<TriangleData> genRandomTriangleData();
        std::vector<TextureData> genFileTextureData();
        std::vector<TextureData> genRandomTextureData();
    };
}

#endif // DATA_GENERATOR_H