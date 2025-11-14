#ifndef COLOR_H
#define COLOR_H

namespace GLRhi
{
    /**
     * @brief 颜色类，用于表示RGBA颜色值
     *
     * 封装了RGBA颜色的存储和操作，提供颜色创建、修改、混合等功能。
     */
    class Color
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

        /**
         * @brief 复制构造函数
         * @param other 要复制的Color对象
         */
        Color(const Color& other);

        /**
         * @brief 赋值运算符
         * @param other 要赋值的Color对象
         * @return 引用自身，支持链式调用
         */
        Color& operator=(const Color& other);

        /**
         * @brief 相等运算符
         * @param other 要比较的Color对象
         * @return 如果所有颜色相等则返回true，否则返回false
         */
        bool operator==(const Color& other) const;

        /**
         * @brief 不等运算符
         * @param other 要比较的Color对象
         * @return 如果颜色不相等则返回true，否则返回false
         */
        bool operator!=(const Color& other) const;

        /**
         * @brief 设置RGBA颜色值
         * @param red 红色 (0.0-1.0)
         * @param green 绿色 (0.0-1.0)
         * @param blue 蓝色 (0.0-1.0)
         * @param alpha 透明度 (0.0-1.0)，默认为1.0
         */
        void set(float red, float green, float blue, float alpha = 1.0f);

        /**
         * @brief 设置RGB颜色值（保持当前透明度）
         * @param red 红色 (0.0-1.0)
         * @param green 绿色 (0.0-1.0)
         * @param blue 蓝色 (0.0-1.0)
         */
        void setRgb(float red, float green, float blue);

        /**
         * @brief 获取RGB颜色值
         * @param red 用于存储红色的引用
         * @param green 用于存储绿色的引用
         * @param blue 用于存储蓝色的引用
         */
        void getRgb(float& red, float& green, float& blue) const;

        /**
         * @brief 获取RGBA颜色值
         * @param red 用于存储红色的引用
         * @param green 用于存储绿色的引用
         * @param blue 用于存储蓝色的引用
         * @param alpha 用于存储透明度的引用
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

    private:
        float m_arrColor[COLOR_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA颜色数组
    };
}

#endif // COLOR_H