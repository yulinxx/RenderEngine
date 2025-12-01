#ifndef RENDERBRUSH_H
#define RENDERBRUSH_H

#include "GLRenderExport.h"
#include "Color.h"

namespace GLRhi
{
    class GLRENDER_API Brush
    {
    public:
        Brush(float red = 1.0f, float green = 1.0f, float blue = 1.0f,
            float alpha = 1.0f, float depth = 0.0f, int type = 0);

        Brush(const Color& c, float depth = 0.0f, int type = 0);
        Brush(const Brush& other);
        Brush& operator=(const Brush& other);

        bool operator==(const Brush& other) const;

        bool operator!=(const Brush& other) const;

        void set(float red, float green, float blue, float alpha = 1.0f);

        void setRgb(float red, float green, float blue);

        void getRgb(float& red, float& green, float& blue) const;

        void getRgba(float& red, float& green, float& blue, float& alpha) const;

        float getRed() const;
        float r() const;
        float getGreen() const;
        float g() const;
        float getBlue() const;
        float b() const;
        float getAlpha() const;
        float a() const;
        const Color& getColor() const;

        Color& getColor();

        float getDepth() const;
        float d() const;
        int getType() const;
        int t() const;

        void setRed(float red);
        void setGreen(float green);
        void setBlue(float blue);
        void setAlpha(float alpha);
        void setColor(const Color& color);

        void setDepth(float depth);
        void setType(int type);

        void clampValues();

        Brush blend(const Brush& other, float factor) const;

    private:
        Color m_color;
        float m_depth = 0.0f;
        int m_type = 0;
    };
}
#endif // RENDERBRUSH_H