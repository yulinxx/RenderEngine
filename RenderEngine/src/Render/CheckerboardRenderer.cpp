#include "CheckerboardRenderer.h"
#include "Shader/CheckerboardShader.h" // 包含棋盘格着色器源码
#include <QDebug>

namespace GLRhi
{
    CheckerboardRenderer::CheckerboardRenderer()
    {
        // 初始化默认颜色
        m_colorA = { 1.0f, 1.0f, 1.0f, 1.0f }; // 亮色
        m_colorB = { 0.9f, 0.9f, 0.9f, 1.0f }; // 暗色
    }

    CheckerboardRenderer::~CheckerboardRenderer()
    {
        cleanup();
    }

    bool CheckerboardRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"CheckerboardRenderer initialize: OpenGL functions not available");
            return false;
        }

        // 初始化着色器
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chCheckerboardVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chCheckerboardFS) ||
            !m_program->link())
        {
            qWarning() << "棋盘格着色器链接失败:" << m_program->log();
            deleteProgram(m_program);
            return false;
        }

        // 创建全屏三角形VAO/VBO（覆盖整个视口）
        float vertices[] = {
            -1.0f, -1.0f, // 左下
             3.0f, -1.0f, // 右下（超出视口）
            -1.0f,  3.0f  // 左上（超出视口）
        };
        m_gl->glGenVertexArrays(1, &m_vao);
        m_gl->glGenBuffers(1, &m_vbo);
        m_gl->glBindVertexArray(m_vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glBindVertexArray(0);

        return true;
    }

    void CheckerboardRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || !m_visible || !m_vao)
            return;

        m_gl->glDisable(GL_DEPTH_TEST); // 棋盘格在最底层，不参与深度测试
        m_program->bind();
        // 设置相机矩阵（用于将世界坐标转换到视口）
        m_program->setUniformValue("cameraMat", QMatrix3x3(cameraMat));
        // 设置棋盘格参数
        m_program->setUniformValue("uCellSize", m_boardSize);
        m_program->setUniformValue("uLightColor", QVector3D(m_colorA.r(), m_colorA.g(), m_colorA.b()));
        m_program->setUniformValue("uDarkColor", QVector3D(m_colorB.r(), m_colorB.g(), m_colorB.b()));

        // 绘制全屏三角形（通过片元着色器计算棋盘格）
        m_gl->glBindVertexArray(m_vao);
        m_gl->glDrawArrays(GL_TRIANGLES, 0, 3);
        m_gl->glBindVertexArray(0);

        m_program->release();
        m_gl->glEnable(GL_DEPTH_TEST);
    }

    void CheckerboardRenderer::cleanup()
    {
        if (!m_gl)
            return;

        deleteProgramAndVaoVbo(m_program, m_vao, m_vbo);
        m_gl = nullptr;
    }
}