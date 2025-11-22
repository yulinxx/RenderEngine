#include "Common/Brush.h"
#include <cmath>

namespace GLRhi
{
    Brush::Brush(float red, float green, float blue, float alpha, float depth, int type)
        : m_color(red, green, blue, alpha), m_depth(depth), m_type(type)
    {
    }

    Brush::Brush(const Brush& other)
        : m_color(other.m_color), m_depth(other.m_depth), m_type(other.m_type)
    {
    }

    Brush& Brush::operator=(const Brush& other)
    {
        if (this != &other)
        {
            m_color = other.m_color;
            m_depth = other.m_depth;
            m_type = other.m_type;
        }
        return *this;
    }

    bool Brush::operator==(const Brush& other) const
    {
        const float epsilon = 1e-6f;
        return (m_color == other.m_color &&
            std::abs(m_depth - other.m_depth) < epsilon &&
            m_type == other.m_type);
    }

    bool Brush::operator!=(const Brush& other) const
    {
        return !(*this == other);
    }

    void Brush::set(float red, float green, float blue, float alpha)
    {
        m_color.set(red, green, blue, alpha);
    }

    void Brush::setRgb(float red, float green, float blue)
    {
        m_color.setRgb(red, green, blue);
    }

    void Brush::getRgb(float& red, float& green, float& blue) const
    {
        m_color.getRgb(red, green, blue);
    }

    void Brush::getRgba(float& red, float& green, float& blue, float& alpha) const
    {
        m_color.getRgba(red, green, blue, alpha);
    }

    void Brush::clampValues()
    {
        m_color.clampValues();
    }

    // 混合两个颜色
    Brush Brush::blend(const Brush& other, float factor) const
    {
        // 使用Color类的混合方法
        Color blendedColor = m_color.blend(other.m_color, factor);

        // 创建新的Brush对象，保留当前的深度和类型
        Brush result;
        result.setColor(blendedColor);
        result.setDepth(m_depth); // 深度值不混合
        result.setType(m_type);   // 类型不混合

        return result;
    }

    // Getter方法实现
    float Brush::getRed() const
    {
        return m_color.getRed();
    }
    float Brush::r() const
    {
        return m_color.r();
    }
    float Brush::getGreen() const
    {
        return m_color.getGreen();
    }
    float Brush::g() const
    {
        return m_color.g();
    }
    float Brush::getBlue() const
    {
        return m_color.getBlue();
    }
    float Brush::b() const
    {
        return m_color.b();
    }
    float Brush::getAlpha() const
    {
        return m_color.getAlpha();
    }
    float Brush::a() const
    {
        return m_color.a();
    }
    const Color& Brush::getColor() const
    {
        return m_color;
    }
    Color& Brush::getColor()
    {
        return m_color;
    }
    float Brush::getDepth() const
    {
        return m_depth;
    }
    float Brush::d() const
    {
        return m_depth;
    }
    int Brush::getType() const
    {
        return m_type;
    }
    int Brush::t() const
    {
        return m_type;
    }

    // Setter方法实现
    void Brush::setRed(float red)
    {
        m_color.setRed(red);
    }
    void Brush::setGreen(float green)
    {
        m_color.setGreen(green);
    }
    void Brush::setBlue(float blue)
    {
        m_color.setBlue(blue);
    }
    void Brush::setAlpha(float alpha)
    {
        m_color.setAlpha(alpha);
    }
    void Brush::setColor(const Color& color)
    {
        m_color = color;
    }
    void Brush::setDepth(float depth)
    {
        m_depth = depth;
    }
    void Brush::setType(int type)
    {
        m_type = type;
    }
}