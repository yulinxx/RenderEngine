// InstancedTextureRenderer.cpp
#include "Render/InstancedTextureRenderer.h"
#include <QDebug>

namespace GLRhi
{
    /**
     * @brief 顶点着色器
     *
     * location 0: 单位矩形顶点 (vec2)
     * location 1: 纹理坐标 (vec2)
     * location 2: 实例位置 (vec2)
     * location 3: 实例尺寸 (vec2)
     * location 4: 实例纹理层 (int)
     * location 5: 实例透明度 (float)
     */
    const char* instancedVS = R"(
        #version 330 core
        layout(location = 0) in vec2 position;      // 基础几何：单位矩形
        layout(location = 1) in vec2 texCoord;
        layout(location = 2) in vec2 instancePos;   // 实例属性：位置
        layout(location = 3) in vec2 instanceSize;  // 实例属性：尺寸
        layout(location = 4) in int instanceLayer;  // 实例属性：纹理层
        layout(location = 5) in float instanceAlpha; // 实例属性：透明度
        
        out vec2 v_TexCoord;
        out float v_Alpha;
        flat out int v_Layer;
        
        uniform mat4 cameraMat;
        
        void main() {
            // 变换：单位矩形 * 尺寸 + 位置
            vec2 worldPos = position * instanceSize + instancePos;
            gl_Position = cameraMat * vec4(worldPos, 0.0, 1.0);
            
            // 传递实例数据到片段着色器
            v_TexCoord = texCoord;
            v_Layer = instanceLayer;
            v_Alpha = instanceAlpha;
        }
    )";

    /**
     * @brief 片段着色器
     *
     * uniform sampler2DArray: 纹理数组
     * 使用3D纹理坐标采样：vec3(uv, layerIndex)
     */
    const char* instancedFS = R"(
        #version 330 core
        in vec2 v_TexCoord;
        in float v_Alpha;
        flat in int v_Layer;
        out vec4 fragColor;
        
        uniform sampler2DArray texArray;
        
        void main() {
            // 从纹理数组的指定层采样
            vec4 color = texture(texArray, vec3(v_TexCoord, v_Layer));
            fragColor = vec4(color.rgb, color.a * v_Alpha);
        }
    )";

    bool InstancedTextureRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl) return false;

        // 创建着色器程序
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, instancedVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, instancedFS) ||
            !m_program->link())
        {
            qWarning() << "Instanced shader link failed:" << m_program->log();
            cleanup();
            return false;
        }

        // 获取uniform位置
        m_program->bind();
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_texArrayLoc = m_program->uniformLocation("texArray");
        m_alphaLoc = m_program->uniformLocation("alpha");
        m_program->release();

        // 创建VAO和VBO
        m_gl->glGenVertexArrays(1, &m_vao);
        m_gl->glGenBuffers(1, &m_vbo);  // 用于实例数据

        // 单位矩形顶点数据（基础几何，4个顶点）
        float unitQuad[] = {
            // 位置      纹理坐标
            -0.5f, -0.5f, 0.0f, 0.0f,  // 左下
             0.5f, -0.5f, 1.0f, 0.0f,  // 右下
             0.5f,  0.5f, 1.0f, 1.0f,  // 右上
            -0.5f,  0.5f, 0.0f, 1.0f   // 左上
        };

        // 创建单位矩形VBO（永不改变）
        GLuint quadVBO;
        m_gl->glGenBuffers(1, &quadVBO);
        m_gl->glBindVertexArray(m_vao);

        // 绑定单位矩形数据（location 0,1）
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(unitQuad), unitQuad, GL_STATIC_DRAW);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        // 配置实例VBO（location 2,3,4,5）
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // 实例位置（vec2）
        m_gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InstancedTextureData), (void*)0);
        m_gl->glEnableVertexAttribArray(2);
        m_gl->glVertexAttribDivisor(2, 1);  // 每个实例更新一次

        // 实例尺寸（vec2）
        m_gl->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(InstancedTextureData), (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(3);
        m_gl->glVertexAttribDivisor(3, 1);

        // 实例纹理层（int）
        m_gl->glVertexAttribIPointer(4, 1, GL_INT, sizeof(InstancedTextureData), (void*)(4 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(4);
        m_gl->glVertexAttribDivisor(4, 1);

        // 实例透明度（float）
        m_gl->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstancedTextureData), (void*)(5 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(5);
        m_gl->glVertexAttribDivisor(5, 1);

        m_gl->glBindVertexArray(0);
        m_gl->glDeleteBuffers(1, &quadVBO);  // 不再需要

        return true;
    }

    void InstancedTextureRenderer::setTextureArray(GLuint textureArrayId, int layerCount)
    {
        m_textureArray = textureArrayId;
        m_layerCount = layerCount;
    }

    void InstancedTextureRenderer::updateInstances(const std::vector<InstancedTextureData>& instances)
    {
        if (!m_gl || !m_vao) return;

        m_instanceCount = instances.size();
        if (m_instanceCount == 0) return;

        // 上传实例数据
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            instances.size() * sizeof(InstancedTextureData),
            instances.data(),
            GL_DYNAMIC_DRAW);  // DYNAMIC：每帧可能变化
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstancedTextureRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_instanceCount == 0 || m_textureArray == 0)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_vao);

        // 启用混合
        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 设置相机矩阵
        //if (m_cameraMatLoc >= 0)
        //{
        //    QMatrix4x4 mat = cameraMat ? QMatrix4x4(cameraMat).transposed()
        //        : QMatrix4x4();
        //    m_program->setUniformValue(m_cameraMatLoc, mat);
        //}
        float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        if (m_cameraMatLoc >= 0)
        {
            QMatrix4x4 mat(identity);
            m_program->setUniformValue(m_cameraMatLoc, mat);
        }

        // 绑定纹理数组（只需一次！）
        if (m_texArrayLoc >= 0)
        {
            m_gl->glActiveTexture(GL_TEXTURE0);
            m_gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
            m_program->setUniformValue(m_texArrayLoc, 0);
        }

        // 绘制：1次调用渲染所有实例！
        m_gl->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_instanceCount);

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void InstancedTextureRenderer::cleanup()
    {
        if (!m_gl) return;

        // 注意：不删除纹理数组，由外部管理
        if (m_vao)
        {
            m_gl->glDeleteVertexArrays(1, &m_vao);
            m_gl->glDeleteBuffers(1, &m_vbo);
            m_vao = m_vbo = 0;
        }

        delete m_program;
        m_program = nullptr;
        m_gl = nullptr;
        m_instanceCount = 0;
    }
}