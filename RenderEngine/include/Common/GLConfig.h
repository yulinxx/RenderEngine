#ifndef GLCONFIG_H
#define GLCONFIG_H

#include <QOpenGLFunctions>

// 配置OpenGL版本
// 支持的版本：3_3, 4_1, 4_2, 4_3, 4_4, 4_5, 4_6
#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 5

// 根据版本包含相应的OpenGL函数头文件
#if GL_VERSION_MAJOR == 3 && GL_VERSION_MINOR == 3
    #include <QOpenGLFunctions_3_3_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_3_3_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 1
    #include <QOpenGLFunctions_4_1_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_1_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 2
    #include <QOpenGLFunctions_4_2_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_2_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 3
    #include <QOpenGLFunctions_4_3_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_3_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 4
    #include <QOpenGLFunctions_4_4_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_4_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 5
    #include <QOpenGLFunctions_4_5_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_5_Core
#elif GL_VERSION_MAJOR == 4 && GL_VERSION_MINOR == 6
    #include <QOpenGLFunctions_4_6_Core>
    #define GL_FUNCTIONS QOpenGLFunctions_4_6_Core
#else
    #error "Unsupported OpenGL version. Please define a valid version in GLConfig.h"
#endif

// 定义获取OpenGL函数的宏
#define GET_GL_FUNCTIONS(context) context->versionFunctions<GL_FUNCTIONS>()

#endif // GLCONFIG_H