#include "Shader/TextureShader.h"

const char* chTextureVS = R"(
        #version 330 core
        layout(location = 0) in vec2 aPosition;
        layout(location = 1) in vec2 aTexCoord;

        out vec2 v_TexCoord;
        uniform mat4 uCameraMat;
        uniform float uDepth;

        void main() {
            gl_Position = uCameraMat * vec4(aPosition, (1 - uDepth) / 2.0f, 1.0);
            v_TexCoord = aTexCoord;
        }
)";

const char* chTextureFS = R"(
        #version 330 core
        in vec2 v_TexCoord;
        out vec4 fragColor;

        uniform sampler2D uTex;
        uniform float uAlpha;

        void main() {
            vec4 color = texture(uTex, v_TexCoord);
            fragColor = vec4(color.rgb, color.a * uAlpha);
        }
)";