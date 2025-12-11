#ifndef CHECKERBOARD_RENDERER_H
#define CHECKERBOARD_RENDERER_H

#include "Render/IRenderer.h"
#include "Render/RenderCommon.h"
#include "Common/Brush.h"

namespace GLRhi
{
    class GLRENDER_API CheckerboardRenderer final : public IRenderer
    {
    public:
        CheckerboardRenderer();
        ~CheckerboardRenderer() override;

    public:
        bool initialize(QOpenGLContext* context) override;
        void render(const float* matMVP = nullptr) override;
        void cleanup() override;

    public:
        void setVisible(bool visible);
        bool isVisible() const;
        void setSize(float size);
        void setColors(const Color& colorA, const Color& colorB);

    private:
        GLuint m_nVao = 0;
        GLuint m_nVbo = 0;

        bool m_bVisible = true;     // 棋盘格是否可见

        Color m_colorA;
        Color m_colorB;

        // Uniform
        int m_uCellSizeLoc = -1;    // 单元格大小Uniform位置
        int m_uLightColorLoc = -1;  // 亮色Uniform位置
        int m_uDarkColorLoc = -1;   // 暗色Uniform位置
        float m_uBoardSize = 26.0f; // 棋盘格大小
    };
}
#endif // CHECKERBOARD_RENDERER_H