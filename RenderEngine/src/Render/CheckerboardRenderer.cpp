#include "Render/CheckerboardRenderer.h"
#include "Shader/CheckerboardShader.h"
#include <QDebug>

namespace GLRhi
{
    CheckerboardRenderer::CheckerboardRenderer()
    {
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
            assert(false && "CheckerboardRenderer initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chCheckerboardVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chCheckerboardFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "CheckerboardRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCellSizeLoc = m_program->uniformLocation("uCellSize");
        m_uLightColorLoc = m_program->uniformLocation("uLightColor");
        m_uDarkColorLoc = m_program->uniformLocation("uDarkColor");
        m_nCameraMatLoc = m_program->uniformLocation("uMVP");

        bool bUniformError = (m_uCellSizeLoc < 0) || (m_uLightColorLoc < 0) || (m_uDarkColorLoc < 0);
        if (bUniformError)
        {
            assert(false && "CheckerboardRenderer: Failed to get uniform locations");
            deleteProgram(m_program);
            return false;
        }

        m_program->release();

        float vertices[] = {
            -1.0f, -1.0f,
             3.0f, -1.0f,
            -1.0f,  3.0f
        };
        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindVertexArray(m_nVao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            assert(false && "CheckerboardRenderer: OpenGL error during initialization");
            cleanup();
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void CheckerboardRenderer::render(const float* mvpMatrix)
    {
        if (!m_gl || !m_program || !m_bVisible || !m_nVao)
            return;

        m_gl->glDisable(GL_DEPTH_TEST); // 棋盘格在最底层
        m_program->bind();

        const QVector3D lightColor(m_colorA.r(), m_colorA.g(), m_colorA.b());
        const QVector3D darkColor(m_colorB.r(), m_colorB.g(), m_colorB.b());

        if (m_nCameraMatLoc >= 0)
            m_program->setUniformValue(m_nCameraMatLoc, QMatrix4x4(mvpMatrix));
        if (m_uCellSizeLoc >= 0)
            m_program->setUniformValue(m_uCellSizeLoc, m_uBoardSize);
        if (m_uLightColorLoc >= 0)
            m_program->setUniformValue(m_uLightColorLoc, lightColor);
        if (m_uDarkColorLoc >= 0)
            m_program->setUniformValue(m_uDarkColorLoc, darkColor);

        m_gl->glBindVertexArray(m_nVao);
        m_gl->glDrawArrays(GL_TRIANGLES, 0, 3);
        m_gl->glBindVertexArray(0);

        m_program->release();
        m_gl->glEnable(GL_DEPTH_TEST);
    }

    void CheckerboardRenderer::cleanup()
    {
        if (!m_gl)
            return;

        deleteProgramAndVaoVbo(m_program, m_nVao, m_nVbo);
        m_gl = nullptr;
    }

    void CheckerboardRenderer::setVisible(bool visible)
    {
        m_bVisible = visible;
    }

    bool CheckerboardRenderer::isVisible() const
    {
        return m_bVisible;
    }

    void CheckerboardRenderer::setSize(float size)
    {
        m_uBoardSize = size;
    }

    void CheckerboardRenderer::setColors(const Color& colorA, const Color& colorB)
    {
        m_colorA = colorA;
        m_colorB = colorB;
    }
}