#include "Render/ImageRenderer.h"
#include "Shader/ImageShader.h"
#include <QDebug>

namespace GLRhi
{
    ImageRenderer::~ImageRenderer()
    {
        cleanup();
    }

    bool ImageRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "ImageRenderer initialize: OpenGL functions not available");
            return false;
        }

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVbo);

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);

        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindVertexArray(0);

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chImageVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chImageFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "ImageRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uTextureLoc = m_program->uniformLocation("uTex");
        m_uDepthLoc = m_program->uniformLocation("uDepth");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uTextureLoc < 0) || (m_uDepthLoc < 0);
        if (bUniformError)
        {
            cleanup();
            assert(false && "ImageRenderer: Failed to get uniform locations");
            return false;
        }

        if (m_uTextureLoc >= 0)
            m_program->setUniformValue(m_uTextureLoc, 0);

        m_program->release();

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "ImageRenderer: OpenGL error during initialization");
            return false;
        }

        return true;
    }

    void ImageRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_vImageInfos.empty() || !m_nVao || !m_nVbo)
            return;

        m_gl->glDisable(GL_CULL_FACE);
        m_program->bind();

        if (m_uCameraMatLoc >= 0)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix3x3(cameraMat));

        m_gl->glBindVertexArray(m_nVao);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);

        for (const auto& img : m_vImageInfos)
        {
            if (!img.textureId || img.vertexCount == 0)
                continue;

            if (m_uDepthLoc >= 0)
                m_program->setUniformValue(m_uDepthLoc, img.depth);

            m_gl->glBufferData(GL_ARRAY_BUFFER, img.vertices.size() * sizeof(float),
                img.vertices.data(), GL_DYNAMIC_DRAW);

            m_gl->glActiveTexture(GL_TEXTURE0);
            m_gl->glBindTexture(GL_TEXTURE_2D, img.textureId);

            m_gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(img.vertexCount));
            m_gl->glBindTexture(GL_TEXTURE_2D, 0);
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

        for (const auto& img : m_vImageInfos)
        {
            if (img.textureId)
                m_gl->glDeleteTextures(1, &img.textureId);
        }
        m_vImageInfos.clear();

        deleteVaoVbo(m_nVao, m_nVbo);
        deleteProgram(m_program);

        m_gl = nullptr;
    }

    void ImageRenderer::updateData(const float* vertices, size_t vertexCount,
        const unsigned char* imgData, int width, int height,
        float depth)
    {
        if (!m_gl || !vertices || vertexCount == 0 || !imgData || width <= 0 || height <= 0)
            return;

        GLuint textureId;
        m_gl->glGenTextures(1, &textureId);
        m_gl->glBindTexture(GL_TEXTURE_2D, textureId);

        m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, imgData);

        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_gl->glBindTexture(GL_TEXTURE_2D, 0);

        ImageInfo img;
        img.textureId = textureId;
        img.vertexCount = vertexCount;
        img.vertices.assign(vertices, vertices + vertexCount * 4); // 每个顶点4个float（x,y,u,v）
        img.depth = depth;                                         // 设置图片深度值
        m_vImageInfos.push_back(std::move(img));
    }
}