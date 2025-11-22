#include "Shader/InstanceTextureShader.h"

const char* chInstanceVS = R"(
        #version 330 core

        layout(location = 0) in vec2 aPosition;
        layout(location = 1) in vec2 aTexCoord;

        layout(location = 2) in vec2 aInstancePos;
        layout(location = 3) in vec2 aInstanceSize;
        layout(location = 4) in int aInstanceLayer;
        layout(location = 5) in float aInstanceAlpha;

        out vec2 v_TexCoord;
        out float v_Alpha;
        flat out int v_Layer;

        uniform mat4 uCameraMat;

        void main() {
            vec2 worldPos = aPosition * aInstanceSize + aInstancePos;
            gl_Position = vec4(worldPos, 0.0, 1.0) * uCameraMat;

            v_TexCoord = aTexCoord;
            v_Layer = aInstanceLayer;
            v_Alpha = aInstanceAlpha;
        }
)";

const char* chInstanceFS = R"(
        #version 330 core

        in vec2 v_TexCoord;
        in float v_Alpha;

        flat in int v_Layer;

        out vec4 fragColor;

        uniform sampler2DArray uTexArray;

        void main() {
            vec4 color = texture(uTexArray, vec3(v_TexCoord, v_Layer));
            fragColor = vec4(color.rgb, color.a * v_Alpha);
        }
)";