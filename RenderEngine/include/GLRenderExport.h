#ifndef GLRENDEREXPORT_H
#define GLRENDEREXPORT_H

#ifdef WIN32
    #ifdef OPENGL_RENDER_DLL_EXPORTS
        #define GLRENDER_EXPORT __declspec(dllexport)
    #else
        #define GLRENDER_EXPORT __declspec(dllimport)
    #endif
#else
    #define GLRENDER_EXPORT
#endif

#endif // GLRENDEREXPORT_H