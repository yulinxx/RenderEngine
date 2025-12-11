#include "Render/TextureRenderer.h"
#include "Shader/TextureShader.h"
#include <QDebug>

namespace GLRhi
{
    TextureRenderer::~TextureRenderer()
    {
        cleanup();
    }

    bool TextureRenderer::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "LineRenderer::initialize: context is null");
            return false;
        }
        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            assert(false && "TextureRenderer::initialize: OpenGL functions not available");
            return false;
        }
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chTextureVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chTextureFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "TextureRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uDepthLoc = m_program->uniformLocation("uDepth");
        m_uTexLoc = m_program->uniformLocation("uTex");
        m_uAlphaLoc = m_program->uniformLocation("uAlpha");

        // bool bUniformError = (m_uCameraMatLoc < 0) || (m_uDepthLoc < 0) || (m_uTexLoc < 0) || (m_uAlphaLoc < 0);
        // if (bUniformError)
        // {
        //     cleanup();
        //     assert(false && "TextureRenderer: Failed to get uniform locations");
        //     return false;
        // }

        m_program->release();

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glGenBuffers(1, &m_nEbo);

        if (m_nVao == 0 || m_nVbo == 0 || m_nEbo == 0)
        {
            cleanup();
            assert(false && "TextureRenderer: Failed to create VAO, VBO or EBO");
            return false;
        }

        m_gl->glBindVertexArray(m_nVao);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "TextureRenderer: OpenGL error during initialization");
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void TextureRenderer::updateData(const std::vector<TextureData>& vTexDatas)
    {
        if (!m_gl || !m_nVao)
            return;

        for (GLuint texId : m_vTextureIds)
        {
            if (texId > 0)
                m_gl->glDeleteTextures(1, &texId);
        }

        m_vBatches.clear();
        m_vTextureIds.clear();
        size_t totalVertices = 0, totalIndices = 0;

        for (const auto& texData : vTexDatas)
        {
            Batch batch;
            batch.indexOffset = static_cast<unsigned int>(totalIndices);
            batch.indexCount = static_cast<unsigned int>(texData.vIndices.size());
            batch.tex = texData.tex;
            batch.brush = texData.brush;
            m_vBatches.push_back(batch);

            m_vTextureIds.push_back(texData.tex);

            totalVertices += texData.vVerts.size() / 4;
            totalIndices += texData.vIndices.size();
        }

        std::vector<float> allVertices;
        allVertices.reserve(totalVertices * 4);
        for (const auto& texData : vTexDatas)
        {
            allVertices.insert(allVertices.end(),
                texData.vVerts.begin(), texData.vVerts.end());
        }

        std::vector<unsigned int> allIndices;
        allIndices.reserve(totalIndices);

        unsigned int vertexOffset = 0;
        for (const auto& texData : vTexDatas)
        {
            for (unsigned int idx : texData.vIndices)
            {
                allIndices.push_back(idx + vertexOffset);
            }
            vertexOffset += static_cast<unsigned int>(texData.vVerts.size() / 4);
        }

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
            allVertices.data(), GL_STATIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int),
            allIndices.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
    }

    void TextureRenderer::render(const float* matMVP)
    {
        if (!m_gl || !m_program || m_vBatches.empty())
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (m_uCameraMatLoc >= 0 && matMVP)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(matMVP));

        m_gl->glActiveTexture(GL_TEXTURE0);
        m_program->setUniformValue(m_uTexLoc, 0);

        for (const auto& batch : m_vBatches)
        {
            m_gl->glBindTexture(GL_TEXTURE_2D, batch.tex);

            if (m_uAlphaLoc >= 0)
                m_program->setUniformValue(m_uAlphaLoc, batch.brush.a());

            if (m_uDepthLoc >= 0)
                m_program->setUniformValue(m_uDepthLoc, batch.brush.d());

            m_gl->glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT,
                (void*)(batch.indexOffset * sizeof(unsigned int)));
        }

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void TextureRenderer::cleanup()
    {
        if (!m_gl)
            return;

        for (GLuint texId : m_vTextureIds)
        {
            if (texId > 0)
                m_gl->glDeleteTextures(1, &texId);
        }

        m_vTextureIds.clear();
        m_vBatches.clear();

        deleteEbo(m_nEbo);
        deleteVaoVbo(m_nVao, m_nVbo);
        deleteProgram(m_program);

        m_gl = nullptr;
    }
}