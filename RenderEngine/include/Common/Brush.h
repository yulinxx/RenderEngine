#ifndef RENDERBRUSH_H
#define RENDERBRUSH_H

#include "GLRenderExport.h"
#include "Common/Color.h"
#include <QImage>
#include <QRectF>
#include <QPointF>
#include <functional>
#include <vector>

namespace GLRhi
{
    /**
     * @brief 渲染画笔类，用于表示颜色、透明度、深度和类型信息
     *
     * 这个类封装了渲染过程中使用的基本属性，包括Color对象（含RGBA颜色值）、深度值和类型标识符。
     * 提供了完整的构造、访问和修改方法，支持颜色操作和比较功能。
     */
    class GLRENDER_EXPORT Brush
    {
    public:
        /**
         * @brief 构造函数
         * @param red 红色 (0.0-1.0)
         * @param green 绿色 (0.0-1.0)
         * @param blue 蓝色 (0.0-1.0)
         * @param alpha 透明度 (0.0-1.0)
         * @param depth 深度值
         * @param type 画笔类型
         */
        Brush(float red = 1.0f, float green = 1.0f, float blue = 1.0f,
            float alpha = 1.0f, float depth = 0.0f, int type = 0);

        /**
         * @brief 复制构造函数
         * @param other 要复制的Brush对象
         */
        Brush(const Brush& other);

        /**
         * @brief 赋值运算符
         * @param other 要赋值的Brush对象
         * @return 引用自身，支持链式调用
         */
        Brush& operator=(const Brush& other);

        /**
         * @brief 相等运算符
         * @param other 要比较的Brush对象
         * @return 如果所有属性相等则返回true，否则返回false
         */
        bool operator==(const Brush& other) const;

        /**
         * @brief 不等运算符
         * @param other 要比较的Brush对象
         * @return 如果属性不相等则返回true，否则返回false
         */
        bool operator!=(const Brush& other) const;

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

        // Getter方法
        float getRed() const;
        float r() const;
        float getGreen() const;
        float g() const;
        float getBlue() const;
        float b() const;
        float getAlpha() const;
        float a() const;

        /**
         * @brief 获取颜色对象
         * @return 颜色对象的常量引用
         */
        const Color& getColor() const;

        /**
         * @brief 获取颜色对象
         * @return 颜色对象的引用
         */
        Color& getColor();

        float getDepth() const;
        float d() const;
        int getType() const;
        int t() const;

        // Setter方法
        void setRed(float red);
        void setGreen(float green);
        void setBlue(float blue);
        void setAlpha(float alpha);

        /**
         * @brief 设置颜色对象
         * @param color 颜色对象
         */
        void setColor(const Color& color);

        void setDepth(float depth);
        void setType(int type);

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
        Brush blend(const Brush& other, float factor) const;

    private:
        Color m_color;         ///< 颜色对象（包含RGBA）
        float m_depth = 0.0f;  ///< 深度值
        int m_type = 0;        ///< 画笔类型
    };
}
#endif // RENDERBRUSH_H