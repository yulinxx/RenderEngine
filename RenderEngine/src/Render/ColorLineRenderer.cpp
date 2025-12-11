#include "Render/ColorLineRenderer.h"
#include "Shader/PathShader.h"
#include "Shader/BaseLineShader.h"
#include <QDebug>

namespace GLRhi
{
    bool ColorLineRenderer::initialize(QOpenGLContext* context)
    {
        m_gl = context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            assert(false && "ColorLineRenderer::initialize: OpenGL functions not available");
            return false;
        }
        m_context = context;

        m_program = new QOpenGLShaderProgram;

        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseLineFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "ColorLineRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uDepthLoc = m_program->uniformLocation("uDepth");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uColorLoc < 0) || (m_uDepthLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "ColorLineRenderer: Failed to get uniform locations");
            return false;
        }

        m_program->release();

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "ColorLineRenderer: OpenGL error during initialization");
            return false;
        }

        return true;
    }

    void ColorLineRenderer::render(const float* matMVP)
    {
        if (!m_gl || !m_program || m_vIndices.empty() || m_vPlineInfos.empty())
            return;

        m_program->bind();

        if (m_uCameraMatLoc >= 0 && matMVP)
        {
            // QMatrix3x3内部是按列优先存储的，与OpenGL兼容
            // matMVP已经是按列优先顺序存储的（MatToFloat函数的输出）
            QMatrix3x3 mat;
            mat(0, 0) = matMVP[0]; mat(1, 0) = matMVP[1]; mat(2, 0) = matMVP[2]; // 第一列
            mat(0, 1) = matMVP[3]; mat(1, 1) = matMVP[4]; mat(2, 1) = matMVP[5]; // 第二列
            mat(0, 2) = matMVP[6]; mat(1, 2) = matMVP[7]; mat(2, 2) = matMVP[8]; // 第三列
            m_program->setUniformValue(m_uCameraMatLoc, mat);
        }

        m_gl->glEnable(GL_PRIMITIVE_RESTART);
        m_gl->glPrimitiveRestartIndex(0xFFFFFFFF);

        m_gl->glBindVertexArray(m_nVao);
        size_t indexOffset = 0;
        const size_t maxIndices = m_vIndices.size();

        size_t polylineCount = 0;
        const size_t maxPolylineCount = m_vPlineInfos.size();

        while (polylineCount < maxPolylineCount && indexOffset < maxIndices)
        {
            const auto& polyline = m_vPlineInfos[polylineCount];
            if (m_uColorLoc >= 0)
                m_program->setUniformValue(m_uColorLoc,
                    QVector4D(polyline.brush.r(), polyline.brush.g(), polyline.brush.b(), polyline.brush.a()));
            if (m_uDepthLoc >= 0)
                m_program->setUniformValue(m_uDepthLoc, polyline.brush.d());

            // 查找当前多段线的索引数量（从当前偏移位置开始，直到找到图元重启索引）
            size_t indexCount = 0;
            size_t maxIterations = maxIndices - indexOffset;

            while (indexCount < maxIterations &&
                indexOffset + indexCount < maxIndices &&
                m_vIndices[indexOffset + indexCount] != 0xFFFFFFFF)
            {
                indexCount++;
            }

            if (indexOffset + indexCount < maxIndices && m_vIndices[indexOffset + indexCount] == 0xFFFFFFFF)
            {
                indexCount += 1;
            }

            if (indexCount > 0 && indexOffset + indexCount <= maxIndices)
            {
                m_gl->glDrawElements(GL_LINE_STRIP,
                    static_cast<GLsizei>(indexCount),
                    GL_UNSIGNED_INT,
                    (void*)(indexOffset * sizeof(GLuint)));
                indexOffset += indexCount;
            }

            polylineCount++;
        }

        m_gl->glDisable(GL_PRIMITIVE_RESTART);
        m_gl->glBindVertexArray(0);

        m_program->release();
    }

    void ColorLineRenderer::cleanup()
    {
        if (!m_gl)
            return;

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        deleteVaoVbo(m_nVao, m_nVbo);
        if (m_nEbo)
        {
            m_gl->glDeleteBuffers(1, &m_nEbo);
            m_nEbo = 0;
        }

        deleteProgram(m_program);
        m_gl = nullptr;
    }

    void ColorLineRenderer::updateData(const std::vector<PolylineData>& polylines)
    {
        if (!m_gl || polylines.empty())
            return;

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        deleteVaoVbo(m_nVao, m_nVbo);

        if (m_nEbo)
        {
            m_gl->glDeleteBuffers(1, &m_nEbo);
            m_nEbo = 0;
        }

        m_vPlineInfos.clear();
        m_vIndices.clear();

        std::vector<float> vVertices;
        size_t nVertexOffset = 0;

        for (const auto& polyline : polylines)
        {
            size_t vertexSz = polyline.vVerts.size();
            if (vertexSz % 3 != 0 || vertexSz == 0)
            {
                qWarning() << "Invalid vertex data: size=" << vertexSz;
                continue;
            }

            m_vPlineInfos.push_back(polyline);

            size_t count = vertexSz / 3;
            vVertices.insert(vVertices.end(), polyline.vVerts.begin(), polyline.vVerts.end());

            for (size_t i = 0; i < count; ++i)
                m_vIndices.push_back(static_cast<GLuint>(nVertexOffset + i));

            m_vIndices.push_back(0xFFFFFFFF);
            nVertexOffset += count;
        }

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vVertices.size() * sizeof(float), vVertices.data(), GL_STATIC_DRAW);

        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(GLuint), m_vIndices.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}