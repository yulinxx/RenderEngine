#ifndef IRENDERER_H
#define IRENDERER_H

#include "GLRenderExport.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include "Common/Brush.h"

namespace GLRhi
{
#define CHECK_GL_ERROR() \
    { \
        GLenum err = m_gl->glGetError(); \
        if (err != GL_NO_ERROR) { \
            qWarning() << "GL Error:" << err << "at" << __FILE__ << ":" << __LINE__; \
        } \
    }

    class GLRENDER_EXPORT IRenderer
    {
    public:
        virtual ~IRenderer() = default;

    public:
        // 初始化资源
        virtual bool initialize(QOpenGLFunctions_3_3_Core* gl) = 0;
        // 渲染
        virtual void render(const float* cameraMat) = 0;
        // 清理资源
        virtual void cleanup() = 0;

    public:
        // 创建 VAO
        GLuint createVao();

        // 创建 VBO
        GLuint createVbo();

        // 创建 EBO
        GLuint createEbo();

        // 绑定 VAO
        void bindVao(GLuint vao);

        // 绑定 VBO
        void bindVbo(GLuint vbo);

        // 绑定 EBO
        void bindEbo(GLuint ebo);

        // 设置 VBO 数据
        void setVboData(GLuint vbo, size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);

        // 设置 EBO 数据
        void setEboData(GLuint ebo, size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);

        // 设置顶点属性
        void setVertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized,
            GLsizei stride, const void* pointer);

        // 启用顶点属性
        void enableVertexAttrib(GLuint index);

        // 解绑VAO/VBO/EBO
        void unbindABE();

        // 删除 VAO
        void deleteVao(GLuint& vao);

        // 删除 VBO
        void deleteVbo(GLuint& vbo);

        // 删除 EBO
        void deleteEbo(GLuint& ebo);

        void deleteProgram(QOpenGLShaderProgram*& p);

        void deleteVaoVbo(GLuint& vao, GLuint& vbo);

        void deleteProgramAndVaoVbo(QOpenGLShaderProgram*& p, GLuint& vao, GLuint& vbo);

    public:
        QOpenGLFunctions_3_3_Core* m_gl = nullptr;
        QOpenGLShaderProgram* m_program = nullptr;

        int m_nColorLoc = -1;       // 颜色Uniform位置
        int m_nDepthLoc = -1;       // 深度
        int m_nCameraMatLoc = -1;   // 相机矩阵
    };
}
#endif // IRENDERER_H