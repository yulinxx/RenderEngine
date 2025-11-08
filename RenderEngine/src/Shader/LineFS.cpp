#include "ShaderDef.h"

const char* lineFS = R"(
    #version 460
    out vec4 outColor;

    void main()
    {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
)";