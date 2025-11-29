#include "Render/LineRendererUbo.h"
#include "Shader/LineUboShader.h"

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QDebug>
#include <array>

namespace GLRhi
{
    LineRendererUbo::~LineRendererUbo()
    {
        cleanup();
    }

    bool LineRendererUbo::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "LineRendererUbo::initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chLineUboVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chLineUboFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "LineRendererUbo: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uCameraMatLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "LineRendererUbo: Failed to get uniform locations");
            return false;
        }

        m_gl->glGenBuffers(1, &m_nUbo);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            deleteProgram(m_program);
            assert(false && "LineRendererUbo: OpenGL error during initialization");
            return false;
        }

        m_program->release();
        return true;
    }

    void LineRendererUbo::cleanup()
    {
        if (!m_gl) return;

        deleteVbo(m_nVbo);
        deleteEbo(m_nEbo);
        deleteVao(m_nVao);
        deleteProgram(m_program);

        if (m_nUbo)
        {
            m_gl->glDeleteBuffers(1, &m_nUbo);
            m_nUbo = 0;
        }

        m_totalIndexCount = 0;
        m_vecBatches.clear();
        m_gl = nullptr;
    }

    void LineRendererUbo::render(const float* cameraMat)
    {
        if (!m_nVao || !m_totalIndexCount || !m_program)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        GLuint nBlockIndex = m_gl->glGetUniformBlockIndex(m_program->programId(), "LineDataUBO");
        if (nBlockIndex != GL_INVALID_INDEX)
        {
            m_gl->glUniformBlockBinding(m_program->programId(), nBlockIndex, 0);
            m_gl->glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_nUbo);
        }

        if (m_uCameraMatLoc >= 0)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(cameraMat));

        m_gl->glDisable(GL_DEPTH_TEST);
        m_gl->glEnable(GL_LINE_SMOOTH);

        m_gl->glEnable(GL_PRIMITIVE_RESTART);
        m_gl->glPrimitiveRestartIndex(0xFFFFFFFF);
        m_gl->glDrawElements(GL_LINE_STRIP, m_totalIndexCount, GL_UNSIGNED_INT, nullptr);
        m_gl->glDisable(GL_PRIMITIVE_RESTART);

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void LineRendererUbo::updateData(const std::vector<PolylineData>& polylines)
    {
        if (!m_gl || polylines.empty())
        {
            cleanup();
            return;
        }

        if (polylines.size() > 2048)
        {
            qCritical() << "UBO count exceeds limit:" << polylines.size();
            return;
        }

        deleteVbo(m_nVbo);
        deleteEbo(m_nEbo);
        deleteVao(m_nVao);

        size_t totalVertices = 0;
        size_t totalIndices = 0;
        for (const auto& p : polylines)
        {
            for (size_t count_idx : p.vCount)
            {
                int count = static_cast<int>(count_idx);
                if (count >= 2)
                {
                    totalVertices += count;
                    totalIndices += count + 1;
                }
            }
        }

        if (totalVertices == 0)
        {
            cleanup();
            return;
        }

        m_totalIndexCount = 0;
        m_vecBatches.clear();

        std::vector<float> vertexData;
        std::vector<GLuint> indexData;
        vertexData.reserve(totalVertices * 4);
        indexData.reserve(totalIndices);

        std::vector<float> uboColors(polylines.size() * 4);
        std::vector<float> uboDepths(polylines.size());

        GLuint vertexOffset = 0;
        GLuint indexOffset = 0;

        for (size_t groupID = 0; groupID < polylines.size(); ++groupID)
        {
            const auto& group = polylines[groupID];
            if (group.vCount.empty()) continue;

            uboColors[groupID * 4 + 0] = group.brush.r();
            uboColors[groupID * 4 + 1] = group.brush.g();
            uboColors[groupID * 4 + 2] = group.brush.b();
            uboColors[groupID * 4 + 3] = group.brush.a();
            uboDepths[groupID] = group.brush.d();

            int vertexIndexInGroup = 0;
            for (size_t vertex_count_idx : group.vCount)
            {
                int lineVertexCount = static_cast<int>(vertex_count_idx);
                if (lineVertexCount < 2)
                {
                    vertexIndexInGroup += lineVertexCount;
                    continue;
                }

                for (int i = 0; i < lineVertexCount; ++i)
                    indexData.push_back(vertexOffset + static_cast<GLuint>(i));

                indexData.push_back(0xFFFFFFFF);

                m_vecBatches.push_back({
                    static_cast<GLuint>(lineVertexCount + 1),
                    indexOffset,
                    static_cast<GLuint>(groupID)
                    });

                vertexOffset += static_cast<GLuint>(lineVertexCount);
                indexOffset += static_cast<GLuint>(lineVertexCount) + 1;
            }

            // 顶点：位置 + 所属线段组ID
            for (size_t i = 0; i < group.vVerts.size() / 3; ++i)
            {
                vertexData.push_back(group.vVerts[i * 3 + 0]);
                vertexData.push_back(group.vVerts[i * 3 + 1]);
                vertexData.push_back(group.vVerts[i * 3 + 2]);
                vertexData.push_back(static_cast<float>(groupID));
            }
        }

        m_totalIndexCount = static_cast<GLsizei>(indexData.size());

        m_gl->glBindBuffer(GL_UNIFORM_BUFFER, m_nUbo);
        m_gl->glBufferData(GL_UNIFORM_BUFFER, 2048 * 4 * sizeof(float) + 2048 * sizeof(float), nullptr, GL_STATIC_DRAW);
        m_gl->glBufferSubData(GL_UNIFORM_BUFFER, 0, uboColors.size() * sizeof(float), uboColors.data());
        m_gl->glBufferSubData(GL_UNIFORM_BUFFER, 2048 * 4 * sizeof(float), uboDepths.size() * sizeof(float), uboDepths.data());
        m_gl->glBindBuffer(GL_UNIFORM_BUFFER, 0);

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // qDebug() << "LineRenderer: Uploaded" << polylines.size() << "groups,"
        //     << (vertexData.size() / 4) << "vertices," << indexData.size() << "indices";
    }
}