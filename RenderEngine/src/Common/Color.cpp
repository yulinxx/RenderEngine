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
        clampValues();
    }

    Color::Color(const Color& other)
    {
        m_arrColor[RED] = other.m_arrColor[RED];
        m_arrColor[GREEN] = other.m_arrColor[GREEN];
        m_arrColor[BLUE] = other.m_arrColor[BLUE];
        m_arrColor[ALPHA] = other.m_arrColor[ALPHA];
        clampValues();
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
        clampValues();
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

    bool Color::operator<(const Color &other) const
    {
        const float epsilon = 1e-5f;
        
        if (std::abs(m_arrColor[RED] - other.m_arrColor[RED]) < epsilon &&
            std::abs(m_arrColor[GREEN] - other.m_arrColor[GREEN]) < epsilon &&
            std::abs(m_arrColor[BLUE] - other.m_arrColor[BLUE]) < epsilon &&
            std::abs(m_arrColor[ALPHA] - other.m_arrColor[ALPHA]) < epsilon)
        {
            return false;
        }
        
        if (m_arrColor[RED] + epsilon < other.m_arrColor[RED])
            return true;
        if (other.m_arrColor[RED] + epsilon < m_arrColor[RED])
            return false;
            
        if (m_arrColor[GREEN] + epsilon < other.m_arrColor[GREEN])
            return true;
        if (other.m_arrColor[GREEN] + epsilon < m_arrColor[GREEN])
            return false;
            
        if (m_arrColor[BLUE] + epsilon < other.m_arrColor[BLUE])
            return true;
        if (other.m_arrColor[BLUE] + epsilon < m_arrColor[BLUE])
            return false;
            
        return m_arrColor[ALPHA] < other.m_arrColor[ALPHA];
    }

    void Color::set(float red, float green, float blue, float alpha)
    {
        m_arrColor[RED] = red;
        m_arrColor[GREEN] = green;
        m_arrColor[BLUE] = blue;
        m_arrColor[ALPHA] = alpha;
        clampValues();
    }

    void Color::setRgb(float red, float green, float blue)
    {
        m_arrColor[RED] = red;
        m_arrColor[GREEN] = green;
        m_arrColor[BLUE] = blue;
        clampValues();
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
            m_arrColor[i] = std::clamp(m_arrColor[i], 0.0f, 1.0f);
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
        clampValues();
    }
    void Color::setGreen(float green)
    {
        m_arrColor[GREEN] = green;
        clampValues();
    }
    void Color::setBlue(float blue)
    {
        m_arrColor[BLUE] = blue;
        clampValues();
    }
    void Color::setAlpha(float alpha)
    {
        m_arrColor[ALPHA] = alpha;
        clampValues();
    }

    uint32_t Color::toUInt32() const
    {
// 将RGBA颜色值转换为32位整数，格式为0xAARRGGBB
    auto toByte = [](float f) -> uint32_t {        
        return static_cast<uint32_t>(std::lround(f * 255.0f));
    };
    
    return (toByte(m_arrColor[ALPHA]) << 24) |
           (toByte(m_arrColor[BLUE])  << 16) |
           (toByte(m_arrColor[GREEN]) << 8) |
           toByte(m_arrColor[RED]);
    }
    
    // 常用颜色静态常量定义
    const Color Color::BLACK = Color(0.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::WHITE = Color(1.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::RED = Color(1.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::GREEN = Color(0.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::BLUE = Color(0.0f, 0.0f, 1.0f, 1.0f);
    const Color Color::YELLOW = Color(1.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::CYAN = Color(0.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::MAGENTA = Color(1.0f, 0.0f, 1.0f, 1.0f);
    const Color Color::GREY = Color(0.5f, 0.5f, 0.5f, 1.0f);
    const Color Color::LIGHT_GREY = Color(0.75f, 0.75f, 0.75f, 1.0f);
    const Color Color::DARK_GREY = Color(0.25f, 0.25f, 0.25f, 1.0f);
    const Color Color::ORANGE = Color(1.0f, 0.65f, 0.0f, 1.0f);
    const Color Color::PURPLE = Color(0.5f, 0.0f, 0.5f, 1.0f);
    const Color Color::BROWN = Color(0.65f, 0.16f, 0.16f, 1.0f);
    const Color Color::PINK = Color(1.0f, 0.75f, 0.8f, 1.0f);
    const Color Color::LIME = Color(0.75f, 1.0f, 0.0f, 1.0f);
    const Color Color::TEAL = Color(0.0f, 0.5f, 0.5f, 1.0f);
    const Color Color::NAVY = Color(0.0f, 0.0f, 0.5f, 1.0f);
    const Color Color::MAROON = Color(0.5f, 0.0f, 0.0f, 1.0f);
    const Color Color::OLIVE = Color(0.5f, 0.5f, 0.0f, 1.0f);
    const Color Color::SILVER = Color(0.75f, 0.75f, 0.75f, 1.0f);
    const Color Color::TRANSPARENT = Color(0.0f, 0.0f, 0.0f, 0.0f);
    
} // namespace GLRhi