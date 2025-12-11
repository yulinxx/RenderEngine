#ifndef IRENDERER_H
#define IRENDERER_H

#include "Common/DllSet.h"
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

    class GLRENDER_API IRenderer
    {
    public:
        virtual ~IRenderer() = default;

    public:
        // 初始化资源,接受 OpenGL 上下文
        virtual bool initialize(QOpenGLContext* context) = 0;
        // 渲染
        virtual void render(const float* matMVP = nullptr) = 0;
        // 清理资源
        virtual void cleanup() = 0;

    public:
        virtual void clearData()
        {
        }

        // 创建
        GLuint createVao();
        GLuint createVbo();
        GLuint createEbo();

        // 绑定
        void bindVao(GLuint vao);
        void bindVbo(GLuint vbo);
        void bindEbo(GLuint ebo);

        // 设置数据
        void setVboData(GLuint vbo, size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);
        void setEboData(GLuint ebo, size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);
        void setVertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized,
            GLsizei stride, const void* pointer);

        // 启用顶点属性
        void enableVertexAttrib(GLuint index);

        // 解绑VAO/VBO/EBO
        void unbindABE();

        // 删除 
        void deleteProgram(QOpenGLShaderProgram*& p);
        void deleteVao(GLuint& vao);
        void deleteVbo(GLuint& vbo);
        void deleteEbo(GLuint& ebo);
        void deleteVaoVbo(GLuint& vao, GLuint& vbo);
        void deleteProgramAndVaoVbo(QOpenGLShaderProgram*& p, GLuint& vao, GLuint& vbo);

    public:
        QOpenGLFunctions_3_3_Core* m_gl = nullptr;
        QOpenGLShaderProgram* m_program = nullptr;
        QOpenGLContext* m_context = nullptr;

        int m_nColorLoc = -1;       // 颜色Uniform位置
        int m_nDepthLoc = -1;       // 深度
        int m_uCameraMatLoc = -1;   // 相机矩阵
    };
}
#endif // IRENDERER_H