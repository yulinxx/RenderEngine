#include "Common/Color.h"
#include <algorithm>
#include <cmath>

namespace GLRhi
{
    Color::Color(float red, float green, float blue, float alpha)
    {
        m_arrColor[RED] = red;
        m_arrColor[GREEN] = green;
        m_arrColor[BLUE] = blue;
        m_arrColor[ALPHA] = alpha;
    }

    Color::Color(const Color& other)
    {
        m_arrColor[RED] = other.m_arrColor[RED];
        m_arrColor[GREEN] = other.m_arrColor[GREEN];
        m_arrColor[BLUE] = other.m_arrColor[BLUE];
        m_arrColor[ALPHA] = other.m_arrColor[ALPHA];
    }

    Color& Color::operator=(const Color& other)
    {
        if (this != &other)
        {
            m_arrColor[RED] = other.m_arrColor[RED];
            m_arrColor[GREEN] = other.m_arrColor[GREEN];
            m_arrColor[BLUE] = other.m_arrColor[BLUE];
            m_arrColor[ALPHA] = other.m_arrColor[ALPHA];
        }
        return *this;
    }

    bool Color::operator==(const Color& other) const
    {
        const float epsilon = 1e-6f;
        return (std::abs(m_arrColor[RED] - other.m_arrColor[RED]) < epsilon &&
            std::abs(m_arrColor[GREEN] - other.m_arrColor[GREEN]) < epsilon &&
            std::abs(m_arrColor[BLUE] - other.m_arrColor[BLUE]) < epsilon &&
            std::abs(m_arrColor[ALPHA] - other.m_arrColor[ALPHA]) < epsilon);
    }

    bool Color::operator!=(const Color& other) const
    {
        return !(*this == other);
    }

    void Color::set(float red, float green, float blue, float alpha)
    {
        m_arrColor[RED] = red;
        m_arrColor[GREEN] = green;
        m_arrColor[BLUE] = blue;
        m_arrColor[ALPHA] = alpha;
    }

    void Color::setRgb(float red, float green, float blue)
    {
        m_arrColor[RED] = red;
        m_arrColor[GREEN] = green;
        m_arrColor[BLUE] = blue;
    }

    void Color::getRgb(float& red, float& green, float& blue) const
    {
        red = m_arrColor[RED];
        green = m_arrColor[GREEN];
        blue = m_arrColor[BLUE];
    }

    void Color::getRgba(float& red, float& green, float& blue, float& alpha) const
    {
        red = m_arrColor[RED];
        green = m_arrColor[GREEN];
        blue = m_arrColor[BLUE];
        alpha = m_arrColor[ALPHA];
    }

    void Color::clampValues()
    {
        for (int i = 0; i < COLOR_COUNT; ++i)
        {
            m_arrColor[i] = std::max(0.0f, std::min(1.0f, m_arrColor[i]));
        }
    }

    Color Color::blend(const Color& other, float factor) const
    {
        factor = std::max(0.0f, std::min(1.0f, factor));
        float invFactor = 1.0f - factor;

        Color result;
        result.m_arrColor[RED] = m_arrColor[RED] * invFactor + other.m_arrColor[RED] * factor;
        result.m_arrColor[GREEN] = m_arrColor[GREEN] * invFactor + other.m_arrColor[GREEN] * factor;
        result.m_arrColor[BLUE] = m_arrColor[BLUE] * invFactor + other.m_arrColor[BLUE] * factor;
        result.m_arrColor[ALPHA] = m_arrColor[ALPHA] * invFactor + other.m_arrColor[ALPHA] * factor;

        return result;
    }

    float Color::getRed() const
    {
        return m_arrColor[RED];
    }
    float Color::r() const
    {
        return m_arrColor[RED];
    }
    float Color::getGreen() const
    {
        return m_arrColor[GREEN];
    }
    float Color::g() const
    {
        return m_arrColor[GREEN];
    }
    float Color::getBlue() const
    {
        return m_arrColor[BLUE];
    }
    float Color::b() const
    {
        return m_arrColor[BLUE];
    }
    float Color::getAlpha() const
    {
        return m_arrColor[ALPHA];
    }
    float Color::a() const
    {
        return m_arrColor[ALPHA];
    }

    void Color::setRed(float red)
    {
        m_arrColor[RED] = red;
    }
    void Color::setGreen(float green)
    {
        m_arrColor[GREEN] = green;
    }
    void Color::setBlue(float blue)
    {
        m_arrColor[BLUE] = blue;
    }
    void Color::setAlpha(float alpha)
    {
        m_arrColor[ALPHA] = alpha;
    }
} // namespace GLRhi