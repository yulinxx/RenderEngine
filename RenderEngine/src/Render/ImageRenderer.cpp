#include "ImageRenderer.h"
#include "../Shader/TextureShader.h" // 包含纹理着色器源码
#include <QDebug>
namespace GLRhi
{
    bool ImageRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"ImageRenderer initialize: OpenGL functions not available");
            return false;
        }
        // 创建共享VAO/VBO（所有图像共用顶点格式）
        m_gl->glGenVertexArrays(1, &m_vao);
        m_gl->glGenBuffers(1, &m_vbo);
        m_gl->glBindVertexArray(m_vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // 顶点属性：位置（x,y）+ 纹理坐标（u,v）
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindVertexArray(0);

        // 初始化着色器
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chTextureVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chTextureFS) ||
            !m_program->link())
        {
            qWarning() << "Image shader link failed:" << m_program->log();
            cleanup();
            return false;
        }

        // 获取Uniform变量位置
        m_program->bind();
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_program->setUniformValue("texture", 0); // 绑定到纹理单元0
        m_program->release();
        return true;
    }

    void ImageRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_imageInfos.empty() || !m_vao || !m_vbo) return;

        m_gl->glDisable(GL_CULL_FACE); // 确保图像正反面可见
        m_program->bind();
        m_program->setUniformValue(m_cameraMatLoc, QMatrix3x3(cameraMat));
        m_program->setUniformValue("depth", m_depth);

        // 合并所有图像的顶点数据到共享VBO
        std::vector<float> allVertices;
        for (const auto& img : m_imageInfos)
        {
            allVertices.insert(allVertices.end(), img.vertices.begin(), img.vertices.end());
        }
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
            allVertices.data(), GL_DYNAMIC_DRAW);

        // 绑定共享VAO
        m_gl->glBindVertexArray(m_vao);

        // 逐个绘制图像
        size_t offset = 0;
        for (const auto& img : m_imageInfos)
        {
            if (!img.textureId || img.vertexCount == 0) continue;

            // 绑定纹理并绘制
            m_gl->glActiveTexture(GL_TEXTURE0);
            m_gl->glBindTexture(GL_TEXTURE_2D, img.textureId);
            m_gl->glDrawArrays(GL_TRIANGLE_STRIP, offset / 4, img.vertexCount); // 每个顶点4个float
            m_gl->glBindTexture(GL_TEXTURE_2D, 0);

            offset += img.vertexCount * 4 * sizeof(float);
        }

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_program->release();
        m_gl->glEnable(GL_CULL_FACE);
    }

    void ImageRenderer::cleanup()
    {
        if (!m_gl)
            return;

        for (const auto& img : m_imageInfos)
        {
            if (img.textureId)
                m_gl->glDeleteTextures(1, &img.textureId);
        }
        m_imageInfos.clear();

        deleteVaoVbo(m_vao, m_vbo);
        deleteProgram(m_program);

        m_gl = nullptr;
    }

    void ImageRenderer::updateData(const float* vertices, size_t vertexCount,
        const unsigned char* imgData, int width, int height)
    {
        if (!m_gl || !vertices || vertexCount == 0 || !imgData || width <= 0 || height <= 0) return;

        // 创建纹理
        GLuint textureId;
        m_gl->glGenTextures(1, &textureId);
        m_gl->glBindTexture(GL_TEXTURE_2D, textureId);
        // 设置纹理数据（RGBA格式）
        m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, imgData);
        // 设置纹理过滤和环绕方式
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_gl->glBindTexture(GL_TEXTURE_2D, 0);

        // 存储图像信息
        ImageInfo img;
        img.textureId = textureId;
        img.vertexCount = vertexCount;
        img.vertices.assign(vertices, vertices + vertexCount * 4); // 每个顶点4个float（x,y,u,v）
        m_imageInfos.push_back(std::move(img));
    }
}