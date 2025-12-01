#ifndef GLRENDEREXPORT_H
#define GLRENDEREXPORT_H

#ifdef WIN32
    #ifdef OPENGL_RENDER_DLL_EXPORTS
        #define GLRENDER_API __declspec(dllexport)
    #else
        #define GLRENDER_API __declspec(dllimport)
    #endif
#else
    // #define GLRENDER_API
    #define GLRENDER_API __attribute__((visibility("default")))
#endif

#endif // GLRENDEREXPORT_H