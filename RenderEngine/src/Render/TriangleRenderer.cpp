#include "Render/TriangleRenderer.h"
#include "Shader/BaseTriangleShader.h"

#include <QDebug>
#include <cassert>

namespace GLRhi
{
    TriangleRenderer::~TriangleRenderer()
    {
        cleanup();
    }

    bool TriangleRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "TriangleRenderer: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseTriangleVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseTriangleFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "TriangleRenderer: Shader link failed");
            return false;
        }

        m_program->bind();

        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uDepthLoc = m_program->uniformLocation("uDepth");
        m_uColorLoc = m_program->uniformLocation("uColor");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uDepthLoc < 0) || (m_uColorLoc < 0);
        if (bUniformError)
        {
            cleanup();
            assert(false && "TriangleRenderer: Failed to get uniform locations");
            return false;
        }

        m_program->release();

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glGenBuffers(1, &m_nEbo);

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "TriangleRenderer: OpenGL error during initialization");
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void TriangleRenderer::updateData(std::vector<TriangleData>& vTriDatas)
    {
        if (!m_gl || !m_nVao)
            return;

        //std::sort(vTriDatas.begin(), vTriDatas.end(), [](const TriangleData& a, const TriangleData& b) {
        //    if (a.brush.d() != b.brush.d())
        //        return a.brush.d() < b.brush.d();

        //    return a.brush.a() >= 1.0f && b.brush.a() < 1.0f; }
        //);

        size_t nTotalVertices = 0;
        size_t nTotalIndices = 0;
        m_vecBatches.clear();
        m_vecBatches.reserve(vTriDatas.size());

        for (const auto& triData : vTriDatas)
        {
            Batch batch;
            batch.indexOffset = static_cast<unsigned int>(nTotalIndices);
            batch.indexCount = static_cast<unsigned int>(triData.indices.size());
            batch.brush = triData.brush;
            m_vecBatches.emplace_back(batch);

            nTotalVertices += triData.verts.size() / 3;
            nTotalIndices += triData.indices.size();
        }

        std::vector<float> allVertices;
        allVertices.reserve(nTotalVertices * 3);
        for (const auto& triData : vTriDatas)
            allVertices.insert(allVertices.end(), triData.verts.begin(), triData.verts.end());

        std::vector<unsigned int> allIndices;
        allIndices.reserve(nTotalIndices);

        unsigned int vertexOffset = 0;
        for (const auto& triData : vTriDatas)
        {
            for (unsigned int index : triData.indices)
                allIndices.push_back(index + vertexOffset);

            vertexOffset += static_cast<unsigned int>(triData.verts.size()) / 3;
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

    void TriangleRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_vecBatches.empty())
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);
        m_gl->glEnable(GL_DEPTH_TEST);

        if (m_bBlend)
        {
            m_gl->glEnable(GL_BLEND);
            m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            m_gl->glBlendEquation(GL_FUNC_ADD);
        }
        else
        {
            m_gl->glDisable(GL_BLEND);
        }

        float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        if (m_uCameraMatLoc >= 0)
        {
            QMatrix4x4 mat(identity);
            m_program->setUniformValue(m_uCameraMatLoc, mat);
        }

        for (const auto& batch : m_vecBatches)
        {
            const auto& b = batch.brush;
            m_program->setUniformValue(m_uDepthLoc, b.d());

            if (m_bBlend)
            {
                m_program->setUniformValue(m_uColorLoc,
                    QVector4D(b.r(), b.g(), b.b(), b.a()));
            }
            else
            {
                m_program->setUniformValue(m_uColorLoc,
                    QVector4D(b.r(), b.g(), b.b(), 1.0f));
            }

            m_gl->glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT,
                (void*)(batch.indexOffset * sizeof(unsigned int)));
        }

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void TriangleRenderer::cleanup()
    {
        if (!m_gl)
            return;

        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);
        deleteProgram(m_program);

        m_gl = nullptr;
    }

    void TriangleRenderer::setBlendEnabled(bool enabled)
    {
        m_bBlend = enabled;
    }

    bool TriangleRenderer::isBlendEnabled() const
    {
        return m_bBlend;
    }
}