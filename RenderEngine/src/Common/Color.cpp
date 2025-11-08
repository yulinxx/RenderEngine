#include "Color.h"
#include <algorithm>

namespace GLRhi
{
    Color& Color::operator=(const Color& other)
    {
        if (this != &other)
        {
            m_red = other.m_red;
            m_green = other.m_green;
            m_blue = other.m_blue;
            m_alpha = other.m_alpha;
        }
        return *this;
    }

    bool Color::operator==(const Color& other) const
    {
        const float epsilon = 1e-6f;
        return (std::abs(m_red - other.m_red) < epsilon &&
            std::abs(m_green - other.m_green) < epsilon &&
            std::abs(m_blue - other.m_blue) < epsilon &&
            std::abs(m_alpha - other.m_alpha) < epsilon);
    }

    bool Color::operator!=(const Color& other) const
    {
        return !(*this == other);
    }

    void Color::set(float red, float green, float blue, float alpha)
    {
        m_red = red;
        m_green = green;
        m_blue = blue;
        m_alpha = alpha;
    }

    void Color::setRgb(float red, float green, float blue)
    {
        m_red = red;
        m_green = green;
        m_blue = blue;
    }

    void Color::getRgb(float& red, float& green, float& blue) const
    {
        red = m_red;
        green = m_green;
        blue = m_blue;
    }

    void Color::getRgba(float& red, float& green, float& blue, float& alpha) const
    {
        red = m_red;
        green = m_green;
        blue = m_blue;
        alpha = m_alpha;
    }

    void Color::clampValues()
    {
        m_red = std::max(0.0f, std::min(1.0f, m_red));
        m_green = std::max(0.0f, std::min(1.0f, m_green));
        m_blue = std::max(0.0f, std::min(1.0f, m_blue));
        m_alpha = std::max(0.0f, std::min(1.0f, m_alpha));
    }

    Color Color::blend(const Color& other, float factor) const
    {
        factor = std::max(0.0f, std::min(1.0f, factor));

        float invFactor = 1.0f - factor;
        return Color(
            m_red * invFactor + other.m_red * factor,
            m_green * invFactor + other.m_green * factor,
            m_blue * invFactor + other.m_blue * factor,
            m_alpha * invFactor + other.m_alpha * factor
        );
    }
} // namespace GLRhi