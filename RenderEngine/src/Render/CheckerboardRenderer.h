#ifndef CHECKERBOARD_RENDERER_H
#define CHECKERBOARD_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include "Common/Brush.h"

namespace GLRhi
{
    class CheckerboardRenderer : public IRenderer
    {
    public:
        CheckerboardRenderer();
        ~CheckerboardRenderer() override;
    public:
        bool initialize(QOpenGLFunctions_3_3_Core* gl) override;
        void render(const float* cameraMat) override;
        void cleanup() override;

    public:
        void setVisible(bool visible)
        {
            m_visible = visible;
        }
        bool isVisible()
        {
            return m_visible;
        }
        void setSize(float size)
        {
            m_boardSize = size;
        }
        void setColors(const Color& colorA, const Color& colorB)
        {
            m_colorA = colorA;
            m_colorB = colorB;
        }

    private:
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        bool m_visible = true;
        float m_boardSize = 26.0f;
        Color m_colorA;
        Color m_colorB;
    };
}
#endif // CHECKERBOARD_RENDERER_H