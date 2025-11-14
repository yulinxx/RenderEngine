#include "Render/ColorTriangleRenderer.h"
#include "Shader/PathShader.h"
#include <QDebug>

namespace GLRhi
{
    bool ColorTriangleRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "ColorTriangleRenderer initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chPathVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chPathFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "ColorTriangleRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("cameraMat");
        m_uDepthLoc = m_program->uniformLocation("depth");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uDepthLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "ColorTriangleRenderer: Failed to get uniform locations");
            return false;
        }

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            deleteProgram(m_program);
            assert(false && "ColorTriangleRenderer: OpenGL error during initialization");
            return false;
        }

        m_program->release();
        return true;
    }

    void ColorTriangleRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_vTriDatas.empty())
            return;

        m_program->bind();
        if (m_uCameraMatLoc >= 0)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix3x3(cameraMat));
        if (m_uDepthLoc >= 0)
            m_program->setUniformValue(m_uDepthLoc, m_dDepth);

        for (const auto& data : m_vTriDatas)
        {
            //if (!data.vao || !data.vbo || data.vertexCount == 0)
            //    continue;

            //if (data.vertexCount % 3 != 0)
            //{
            //    qWarning() << "Fill vertex count must be multiple of 3";
            //    continue;
            //}

            //// 设置填充颜色
            //m_program->setUniformValue("color", QVector4D(
            //    data.brush.r(), data.brush.g(), data.brush.b(), 1.0f));

            //// 绑定VAO并绘制（三角形集合）
            //m_gl->glBindVertexArray(data.vao);
            //m_gl->glDrawArrays(GL_TRIANGLES, 0, data.vertexCount);
            //m_gl->glBindVertexArray(0);
        }
        m_program->release();
    }

    void ColorTriangleRenderer::cleanup()
    {
        if (!m_gl) return;

        //for (auto& fill : m_vTriDatas)
        //    deleteVaoVbo(fill.vao, fill.vbo);

        deleteProgram(m_program);

        m_vTriDatas.clear();
        m_gl = nullptr;
    }

    void ColorTriangleRenderer::updateData(float* data, size_t count, const Brush& color)
    {
        //if (!m_gl || !data || count == 0 || count % 3 != 0) return;

        //TriangleData triData;
        //triData.brush = color;
        //triData.vertexCount = count;
        //triData.vertices.assign(data, data + count * 3); // 每个顶点3个float（x,y,depth）

        //m_gl->glGenVertexArrays(1, &triData.vao);
        //m_gl->glGenBuffers(1, &triData.vbo);
        //m_gl->glBindVertexArray(triData.vao);
        //m_gl->glBindBuffer(GL_ARRAY_BUFFER, triData.vbo);
        //m_gl->glBufferData(GL_ARRAY_BUFFER, triData.vertices.size() * sizeof(float),
        //    triData.vertices.data(), GL_STATIC_DRAW);

        //// 顶点属性：位置（x,y）+ 深度（z）
        //m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        //m_gl->glEnableVertexAttribArray(0);
        //m_gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        //    (void*)(2 * sizeof(float)));
        //m_gl->glEnableVertexAttribArray(1);

        //m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        //m_gl->glBindVertexArray(0);

        //m_vTriDatas.push_back(std::move(triData));
    }
}