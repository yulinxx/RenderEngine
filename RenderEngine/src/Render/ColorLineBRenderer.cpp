#include "ColorLineBRenderer.h"
#include "Shader/PathBoldShader.h" // 包含粗线着色器源码
#include <QDebug>

namespace GLRhi
{
    bool ColorLineBRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"ColorLineBRenderer initialize: OpenGL functions not available");
            return false;
        }
        // 初始化着色器
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chPathBoldVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Geometry, chPathBoldGS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chPathBoldFS) ||
            !m_program->link())
        {
            qWarning() << "粗线着色器链接失败:" << m_program->log();
            deleteProgram(m_program);
            return false;
        }

        // 获取Uniform变量位置
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_colorLoc = m_program->uniformLocation("uColor");
        m_lineTypeLoc = m_program->uniformLocation("uLineType");
        m_dashScaleLoc = m_program->uniformLocation("uDashScale");
        m_thicknessLoc = m_program->uniformLocation("uThickness");

        return true;
    }

    void ColorLineBRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program) return;

        m_program->bind();
        m_program->setUniformValue(m_cameraMatLoc, QMatrix3x3(cameraMat));

        for (auto& lineBInfo : m_lineBInfos)
        {
            if (lineBInfo.vao && lineBInfo.vertexCount > 0)
            {
                // 设置线的参数
                m_program->setUniformValue(m_colorLoc,
                    QVector4D(lineBInfo.color.r(), lineBInfo.color.g(), lineBInfo.color.b(), lineBInfo.color.a()));

                m_program->setUniformValue(m_lineTypeLoc, lineBInfo.lineType);
                m_program->setUniformValue(m_dashScaleLoc, lineBInfo.dashScale);
                m_program->setUniformValue(m_thicknessLoc, lineBInfo.thickness);

                // 绘制线段
                m_gl->glBindVertexArray(lineBInfo.vao);
                m_gl->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineBInfo.vertexCount));
                m_gl->glBindVertexArray(0);
            }
        }

        m_program->release();
    }

    void ColorLineBRenderer::cleanup()
    {
        if (!m_gl) return;

        // 清理所有线段数据
        for (auto& lineBInfo : m_lineBInfos)
        {
            deleteVaoVbo(lineBInfo.vao, lineBInfo.vbo);

            lineBInfo.vertices.clear();
            lineBInfo.vertexCount = 0;
        }

        deleteProgram(m_program);

        m_lineBInfos.clear();
        m_gl = nullptr;
    }

    void ColorLineBRenderer::updateData(float* data, size_t count, const Brush& color,
        int lineType, float dashScale, float thickness)
    {
        if (!m_gl || !data || count == 0) return;

        // 创建新的线段信息
        LineBInfo lineBInfo;
        lineBInfo.color = color;
        lineBInfo.lineType = lineType;
        lineBInfo.dashScale = dashScale;
        lineBInfo.thickness = thickness;

        // 复制顶点数据
        size_t vertexCount = count / 3; // 每个顶点3个float: x, y, length
        lineBInfo.vertices.resize(count);
        memcpy(lineBInfo.vertices.data(), data, count * sizeof(float));
        lineBInfo.vertexCount = vertexCount;

        // 创建VAO/VBO
        m_gl->glGenVertexArrays(1, &lineBInfo.vao);
        m_gl->glGenBuffers(1, &lineBInfo.vbo);
        m_gl->glBindVertexArray(lineBInfo.vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, lineBInfo.vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), lineBInfo.vertices.data(), GL_STATIC_DRAW);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glBindVertexArray(0);

        // 添加到线段列表
        m_lineBInfos.push_back(lineBInfo);
    }
}