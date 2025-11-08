#include "IRenderer.h"

namespace GLRhi
{
    // 创建 VAO
    GLuint IRenderer::createVao()
    {
        GLuint vao = 0;
        m_gl->glGenVertexArrays(1, &vao);
        return vao;
    }

    GLuint IRenderer::createVbo()
    {
        GLuint vbo = 0;
        m_gl->glGenBuffers(1, &vbo);
        return vbo;
    }

    // 创建 EBO
    GLuint IRenderer::createEbo()
    {
        GLuint ebo = 0;
        m_gl->glGenBuffers(1, &ebo);
        return ebo;
    }

    // 绑定 VAO
    void IRenderer::bindVao(GLuint vao)
    {
        m_gl->glBindVertexArray(vao);
    }

    void IRenderer::bindVbo(GLuint vbo)
    {
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void IRenderer::bindEbo(GLuint ebo)
    {
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    void IRenderer::setVboData(GLuint vbo, size_t size, const void* data, GLenum usage)
    {
        bindVbo(vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }

    void IRenderer::setEboData(GLuint ebo, size_t size, const void* data, GLenum usage)
    {
        bindEbo(ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }

    void IRenderer::setVertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized,
        GLsizei stride, const void* pointer)
    {
        m_gl->glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    void IRenderer::enableVertexAttrib(GLuint index)
    {
        m_gl->glEnableVertexAttribArray(index);
    }

    void IRenderer::unbindABE()
    {
        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // 删除 VAO
    void IRenderer::deleteVao(GLuint& vao)
    {
        if (vao)
        {
            m_gl->glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }

    // 删除 VBO
    void IRenderer::deleteVbo(GLuint& vbo)
    {
        if (vbo)
        {
            m_gl->glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    // 删除 EBO
    void IRenderer::deleteEbo(GLuint& ebo)
    {
        if (ebo)
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
        if (vao)
        {
            m_gl->glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        if (vbo)
        {
            m_gl->glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    // 删除着色器程序、VAO 和 VBO
    void IRenderer::deleteProgramAndVaoVbo(QOpenGLShaderProgram*& program, GLuint& vao, GLuint& vbo)
    {
        deleteProgram(program);
        deleteVaoVbo(vao, vbo);
    }
}