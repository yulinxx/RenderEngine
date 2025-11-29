#ifndef COLOR_H
#define COLOR_H

#include "GLRenderExport.h"
#include <cstdint>

namespace GLRhi
{
    /**
     * @brief 颜色类，用于表示RGBA颜色值
     *
     * 封装了RGBA颜色的存储和操作，提供颜色创建、修改、混合等功能。
     */
    class GLRENDER_EXPORT Color
    {
    public:
        enum ColorIndex
        {
            RED = 0,
            GREEN = 1,
            BLUE = 2,
            ALPHA = 3,
            COLOR_COUNT = 4
        };

        /**
         * @brief 构造函数
         * @param red 红色 (0.0-1.0)
         * @param green 绿色 (0.0-1.0)
         * @param blue 蓝色 (0.0-1.0)
         * @param alpha 透明度 (0.0-1.0)
         */
        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
        Color(const Color& other);

        Color& operator=(const Color& other);
        bool operator==(const Color& other) const;
        bool operator!=(const Color& other) const;
        bool operator<(const Color& other) const;

        /**
         * @brief 设置RGBA颜色值
         */
        void set(float red, float green, float blue, float alpha = 1.0f);

        /**
         * @brief 设置RGB颜色值（保持当前透明度）
         */
        void setRgb(float red, float green, float blue);

        /**
         * @brief 获取RGB颜色值
         */
        void getRgb(float& red, float& green, float& blue) const;

        /**
         * @brief 获取RGBA颜色值
         */
        void getRgba(float& red, float& green, float& blue, float& alpha) const;

        /**
         * @brief 将颜色值限制在有效范围内 (0.0-1.0)
         */
        void clampValues();

        /**
         * @brief 混合两个颜色
         * @param other 另一个颜色
         * @param factor 混合因子 (0.0-1.0)，0表示完全使用当前颜色，1表示完全使用other颜色
         * @return 混合后的新颜色
         */
        Color blend(const Color& other, float factor) const;

        float getRed() const;
        float r() const;
        float getGreen() const;
        float g() const;
        float getBlue() const;
        float b() const;
        float getAlpha() const;
        float a() const;

        void setRed(float red);
        void setGreen(float green);
        void setBlue(float blue);
        void setAlpha(float alpha);

        uint32_t toUInt32() const;
        
    private:
        float m_arrColor[COLOR_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA颜色数组
    };
}

#endif // COLOR_H