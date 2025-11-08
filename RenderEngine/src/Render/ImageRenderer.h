#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include "IRenderer.h"
#include "RenderCommon.h"

#include <vector>
namespace GLRhi
{


struct ImageInfo {
    GLuint textureId = 0;      // 纹理ID
    std::vector<float> vertices; // 格式: x, y, u, v（每个顶点4个float，含纹理坐标）
    size_t vertexCount = 0;    // 顶点数量（通常为4，对应矩形的4个顶点）
};

class ImageRenderer : public IRenderer {
public:
    ImageRenderer() = default;
    ~ImageRenderer() override { cleanup(); }

    bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
    void render(const float* cameraMat) override;
    void cleanup() override;

    // 更新图像数据（顶点+纹理）
    void updateData(const float* vertices, size_t vertexCount,
                   const unsigned char* imgData, int width, int height);

private:
    std::vector<ImageInfo> m_imageInfos;
    GLuint m_vao = 0;  // 共享VAO（所有图像共用同一顶点格式）
    GLuint m_vbo = 0;  // 共享VBO（动态更新所有图像顶点数据）
    int m_cameraMatLoc = -1; // 相机矩阵Uniform位置
    float m_depth = 0.6f;    // 图像默认深度
};

}
#endif // IMAGERENDERER_H