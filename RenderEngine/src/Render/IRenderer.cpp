#include "Render/IRenderer.h"

namespace GLRhi
{
    GLuint IRenderer::createVao()
    {
        GLuint vao = 0;
        if (m_gl)
            m_gl->glGenVertexArrays(1, &vao);
        return vao;
    }

    GLuint IRenderer::createVbo()
    {
        GLuint vbo = 0;
        if (m_gl)
            m_gl->glGenBuffers(1, &vbo);
        return vbo;
    }

    GLuint IRenderer::createEbo()
    {
        GLuint ebo = 0;
        if (m_gl)
            m_gl->glGenBuffers(1, &ebo);
        return ebo;
    }

    void IRenderer::bindVao(GLuint vao)
    {
        if (m_gl)
            m_gl->glBindVertexArray(vao);
    }

    void IRenderer::bindVbo(GLuint vbo)
    {
        if (m_gl)
            m_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void IRenderer::bindEbo(GLuint ebo)
    {
        if (m_gl)
            m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    void IRenderer::setVboData(GLuint vbo, size_t size, const void* data, GLenum usage)
    {
        bindVbo(vbo);
        if (m_gl)
            m_gl->glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }

    void IRenderer::setEboData(GLuint ebo, size_t size, const void* data, GLenum usage)
    {
        bindEbo(ebo);
        if (m_gl)
            m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }

    void IRenderer::setVertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized,
        GLsizei stride, const void* pointer)
    {
        if (m_gl)
            m_gl->glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    void IRenderer::enableVertexAttrib(GLuint index)
    {
        if (m_gl)
            m_gl->glEnableVertexAttribArray(index);
    }

    void IRenderer::unbindABE()
    {
        if (m_gl)
        {
            m_gl->glBindVertexArray(0);
            m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
            m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    void IRenderer::deleteVao(GLuint& vao)
    {
        if (m_gl && vao)
        {
            m_gl->glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }

    void IRenderer::deleteVbo(GLuint& vbo)
    {
        if (m_gl && vbo)
        {
            m_gl->glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    void IRenderer::deleteEbo(GLuint& ebo)
    {
        if (m_gl && ebo)
        {
            m_gl->glDeleteBuffers(1, &ebo);
            ebo = 0;
        }
    }

    void IRenderer::deleteProgram(QOpenGLShaderProgram*& program)
    {
        if (program)
        {
            delete program;
            program = nullptr;
        }
    }

    void IRenderer::deleteVaoVbo(GLuint& vao, GLuint& vbo)
    {
        if (m_gl && vao)
        {
            m_gl->glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        if (m_gl && vbo)
        {
            m_gl->glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    void IRenderer::deleteProgramAndVaoVbo(QOpenGLShaderProgram*& program, GLuint& vao, GLuint& vbo)
    {
        deleteProgram(program);
        deleteVaoVbo(vao, vbo);
    }
}