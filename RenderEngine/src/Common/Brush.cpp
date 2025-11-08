#include "Brush.h"

namespace GLRhi
{
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
}