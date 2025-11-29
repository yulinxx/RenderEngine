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


    // 常用颜色定义
    static inline const Color BLACK{ 0.0f, 0.0f, 0.0f, 1.0f };       // 黑色
    static inline const Color WHITE{ 1.0f, 1.0f, 1.0f, 1.0f };       // 白色
    static inline const Color RED{ 1.0f, 0.0f, 0.0f, 1.0f };         // 红色
    static inline const Color GREEN{ 0.0f, 1.0f, 0.0f, 1.0f };       // 绿色
    static inline const Color BLUE{ 0.0f, 0.0f, 1.0f, 1.0f };        // 蓝色
    static inline const Color YELLOW{ 1.0f, 1.0f, 0.0f, 1.0f };      // 黄色
    static inline const Color CYAN{ 0.0f, 1.0f, 1.0f, 1.0f };        // 青色
    static inline const Color MAGENTA{ 1.0f, 0.0f, 1.0f, 1.0f };     // 品红色
    static inline const Color GREY{ 0.5f, 0.5f, 0.5f, 1.0f };        // 灰色
    static inline const Color LIGHT_GREY{ 0.75f, 0.75f, 0.75f, 1.0f }; // 浅灰色
    static inline const Color DARK_GREY{ 0.25f, 0.25f, 0.25f, 1.0f };  // 深灰色
    static inline const Color ORANGE{ 1.0f, 0.65f, 0.0f, 1.0f };     // 橙色
    static inline const Color PURPLE{ 0.5f, 0.0f, 0.5f, 1.0f };      // 紫色
    static inline const Color BROWN{ 0.65f, 0.16f, 0.16f, 1.0f };    // 棕色
    static inline const Color PINK{ 1.0f, 0.75f, 0.8f, 1.0f };       // 粉色
    static inline const Color LIME{ 0.75f, 1.0f, 0.0f, 1.0f };       // 酸橙色
    static inline const Color TEAL{ 0.0f, 0.5f, 0.5f, 1.0f };        // 青色/蓝绿色
    static inline const Color NAVY{ 0.0f, 0.0f, 0.5f, 1.0f };        // 藏青色
    static inline const Color MAROON{ 0.5f, 0.0f, 0.0f, 1.0f };      // 栗色
    static inline const Color OLIVE{ 0.5f, 0.5f, 0.0f, 1.0f };       // 橄榄绿
    static inline const Color SILVER{ 0.75f, 0.75f, 0.75f, 1.0f };   // 银色
    //static inline const Color TRANSPARENT{ 0.0f, 0.0f, 0.0f, 0.0f }; // 透明色
}

#endif // COLOR_H