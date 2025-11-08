#include "ColorTriangleRenderer.h"
#include "Shader/PathShader.h"
#include <QDebug>

namespace GLRhi
{
    bool ColorTriangleRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"ColorTriangleRenderer initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;
        // 加载顶点和片段着色器（填充与线段共享基础着色器）
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chPathVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chPathFS) ||
            !m_program->link())
        {
            qWarning() << "Fill shader link failed:" << m_program->log();
            deleteProgram(m_program);
            return false;
        }

        // 获取Uniform变量位置
        m_program->bind();
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_program->release();
        return true;
    }

    void ColorTriangleRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_vTriDatas.empty())
            return;

        m_program->bind();
        m_program->setUniformValue(m_cameraMatLoc, QMatrix3x3(cameraMat));
        m_program->setUniformValue("depth", m_depth);

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

        // 释放所有VAO/VBO
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

        //// 创建VAO/VBO
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