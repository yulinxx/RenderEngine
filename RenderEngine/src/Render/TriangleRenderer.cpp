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
            assert(false && "TriangleRenderer::initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseTriangleVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseTriangleFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "TriangleRenderer: Shader link failed");
            return false;
        }

        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uDepthLoc = m_program->uniformLocation("uDepth");
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uColorLoc < 0) || (m_uDepthLoc < 0) || (m_uCameraMatLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "TriangleRenderer: Failed to get uniform locations");
            return false;
        }

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "TriangleRenderer: OpenGL error during initialization");
            return false;
        }

        return true;
    }

    void TriangleRenderer::render(const float* cameraMat)
    {
        if (m_nVao == 0 || m_nVbo == 0 || m_nEbo == 0 ||
            m_vTriangleBrush.empty() || m_vIndexCounts.empty() || m_vIndexOffsets.empty())
        {
            return;
        }

        if (m_vTriangleBrush.size() != m_vIndexCounts.size() ||
            m_vTriangleBrush.size() != m_vIndexOffsets.size())
        {
            qWarning() << "TriangleRenderer::render: container size mismatch!";
            return;
        }

        m_program->bind();

        if (m_uCameraMatLoc >= 0)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(cameraMat));

        // 设置混合模式
        if (m_bBlend)
        {
            m_gl->glEnable(GL_BLEND);
            m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            m_gl->glDisable(GL_BLEND);
        }

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);

        // 渲染每个三角形组
        for (size_t i = 0; i < m_vTriangleBrush.size(); ++i)
        {
            const auto& brush = m_vTriangleBrush[i];
            if (m_vIndexCounts[i] <= 0)
                continue;

            m_program->setUniformValue(m_uDepthLoc, brush.d());
            m_program->setUniformValue(m_uColorLoc, QVector4D(brush.r(), brush.g(), brush.b(), brush.a()));

            void* byteOffset = reinterpret_cast<void*>(m_vIndexOffsets[i] * sizeof(GLuint));
            m_gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_vIndexCounts[i]), GL_UNSIGNED_INT, byteOffset);
        }

        m_gl->glBindVertexArray(0);

        if (m_bBlend)
        {
            m_gl->glDisable(GL_BLEND);
        }

        m_program->release();
    }

    void TriangleRenderer::cleanup()
    {
        if (!m_gl)
            return;

        unbindABE();
        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);
        deleteProgram(m_program);

        m_vTriangleBrush.clear();
        m_vTriangleBrush.shrink_to_fit();

        m_vIndexOffsets.clear();
        m_vIndexOffsets.shrink_to_fit();

        m_vIndexCounts.clear();
        m_vIndexCounts.shrink_to_fit();

        m_gl = nullptr;
    }

    void TriangleRenderer::updateData(const std::vector<TriangleData>& triangleDatas)
    {
        if (!m_gl || triangleDatas.empty())
        {
            cleanup();
            return;
        }

        unbindABE();
        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);

        m_vTriangleBrush.clear();
        m_vIndexCounts.clear();
        m_vIndexOffsets.clear();

        std::vector<float> vAllVertices;
        std::vector<GLuint> vAllIndices;
        GLuint nVertexOffset = 0;
        GLuint nIndexOffset = 0;

        for (const auto& triangleData : triangleDatas)
        {
            const auto& verts = triangleData.vVerts;
            const auto& indices = triangleData.vIndices;

            if (verts.empty() || verts.size() % 3 != 0)
            {
                qWarning() << "Skipping invalid triangle data: vertex count not divisible by 3";
                continue;
            }

            if (indices.empty() || indices.size() % 3 != 0)
            {
                qWarning() << "Skipping invalid triangle data: index count not divisible by 3";
                continue;
            }

            // 添加画刷信息
            m_vTriangleBrush.push_back(triangleData.brush);

            // 记录当前组的索引偏移和数量
            m_vIndexOffsets.push_back(nIndexOffset);
            m_vIndexCounts.push_back(static_cast<GLuint>(indices.size()));

            // 添加顶点数据
            vAllVertices.insert(vAllVertices.end(), verts.begin(), verts.end());

            // 添加索引数据（需要加上顶点偏移）
            for (GLuint index : indices)
            {
                vAllIndices.push_back(nVertexOffset + index);
            }

            // 更新偏移量
            nVertexOffset += static_cast<GLuint>(verts.size() / 3);
            nIndexOffset += static_cast<GLuint>(indices.size());
        }

        if (vAllVertices.empty() || vAllIndices.empty())
        {
            qWarning() << "TriangleRenderer::updateData: No valid data to render";
            return;
        }

        // 创建VAO
        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        // 创建并填充VBO
        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vAllVertices.size() * sizeof(float), vAllVertices.data(), GL_STATIC_DRAW);

        // 设置顶点属性指针
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);

        // 创建并填充EBO
        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, vAllIndices.size() * sizeof(GLuint), vAllIndices.data(), GL_STATIC_DRAW);

        unbindABE();

        qDebug() << "TriangleRenderer::updateData: Loaded" << triangleDatas.size()
            << "triangle groups with" << (vAllVertices.size() / 3)
            << "vertices and" << (vAllIndices.size() / 3) << "triangles";
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