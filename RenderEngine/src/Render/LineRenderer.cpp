#include "LineRenderer.h"
#include "Shader/PathShader.h"
#include "Shader/BaseLineShader.h"
#include <QDebug>
#include <cassert>

namespace GLRhi
{
    bool LineRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            qCritical() << "LineRenderer::initialize: OpenGL functions not available";
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        // 加载着色器
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseLineFS) ||
            !m_program->link())
        {
            qWarning() << "Line shader link failed:" << m_program->log();
            deleteProgram(m_program);
            return false;
        }

        // 缓存 uniform 位置
        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uDepthLoc = m_program->uniformLocation("uDepth");

        return true;
    }

    void LineRenderer::render(const float* cameraMat)
    {
        if (m_nVao == 0 || m_nVbo == 0 || m_nEbo == 0 ||
            m_vPlineBrush.empty() || m_vIndexCounts.empty() || m_vIndexOffsets.empty())
        {
            return;
        }

        if (m_vPlineBrush.size() != m_vIndexCounts.size() ||
            m_vPlineBrush.size() != m_vIndexOffsets.size())
        {
            qWarning() << "LineRenderer::render: container size mismatch!";
            return;
        }

        m_program->bind();

        // 设置相机矩阵
        //if (m_cameraMatLoc >= 0)
        //{
        //    //m_program->setUniformValue(m_cameraMatLoc, QMatrix4x4(cameraMat));
        //}

        // 启用图元重启
        m_gl->glEnable(GL_PRIMITIVE_RESTART);
        m_gl->glPrimitiveRestartIndex(0xFFFFFFFF);

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);

        for (size_t i = 0; i < m_vPlineBrush.size(); ++i)
        {
            const auto& b = m_vPlineBrush[i];
            if (m_vIndexCounts[i] <= 0)
                continue;

            m_program->setUniformValue(m_uDepthLoc, b.d());
            m_program->setUniformValue(m_uColorLoc,QVector4D(b.r(), b.g(), b.b(), b.a()));

            void* byteOffset = reinterpret_cast<void*>(m_vIndexOffsets[i] * sizeof(GLuint));
            m_gl->glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(m_vIndexCounts[i]), GL_UNSIGNED_INT, byteOffset);
        }

        m_gl->glDisable(GL_PRIMITIVE_RESTART);
        m_gl->glBindVertexArray(0);

        m_program->release();
    }

    void LineRenderer::cleanup()
    {
        if (!m_gl)
            return;

        unbindABE();
        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);
        deleteProgram(m_program);

        // 清空 CPU 数据
        m_vPlineBrush.clear();
        m_vPlineBrush.shrink_to_fit();

        m_vIndexOffsets.clear();
        m_vIndexOffsets.shrink_to_fit();
        
        m_vIndexCounts.clear();
        m_vIndexCounts.shrink_to_fit();
    }

    void LineRenderer::updateData(const std::vector<PolylineData>& vPolylineDatas)
    {
        if (!m_gl || vPolylineDatas.empty())
        {
            cleanup();
            return;
        }

        unbindABE();

        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);

        m_vPlineBrush.clear();
        m_vIndexCounts.clear();
        m_vIndexOffsets.clear();

        std::vector<float> vAllVertices;
        std::vector<GLuint> vIndices;
        GLuint nVertexOffset = 0;
        GLuint nIndexOffsetE = 0;

        // 处理每个polyline数据
        for (const auto& polylineData : vPolylineDatas)
        {
            const auto& allVerts = polylineData.vertices;
            if (allVerts.empty() || allVerts.size() % 3 != 0)
            {
                qWarning() << "Skipping invalid polyline: vertex count not divisible by 3";
                continue;
            }

            m_vPlineBrush.push_back(polylineData.brush);

            m_vIndexOffsets.push_back(nIndexOffsetE);

            vAllVertices.insert(vAllVertices.end(), allVerts.begin(), allVerts.end());

            size_t startIndex = nVertexOffset;
            const auto& lineVertexSz = polylineData.lineVertexCounts;
            for (size_t count : lineVertexSz)
            {
                for (size_t j = 0; j < count; ++j)
                    vIndices.push_back(static_cast<GLuint>(startIndex + j));

                vIndices.push_back(0xFFFFFFFF);
                startIndex += count;
            }

            size_t nAllVertexSz = allVerts.size() / 3;
            m_vIndexCounts.push_back(static_cast<GLuint>(nAllVertexSz));

            nVertexOffset += static_cast<GLuint>(nAllVertexSz);
            nIndexOffsetE += static_cast<GLuint>(nAllVertexSz) + static_cast<GLuint>(lineVertexSz.size());
        }

        if (vAllVertices.empty() || vIndices.empty())
            return;

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vAllVertices.size() * sizeof(float), vAllVertices.data(), GL_STATIC_DRAW);

        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);

        // EBO
        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(GLuint), vIndices.data(), GL_STATIC_DRAW);

        unbindABE();
    }
}