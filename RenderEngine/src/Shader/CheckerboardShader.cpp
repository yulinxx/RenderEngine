const char* chCheckerboardVS = R"(
#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 uMVP;  // Model-View-Projection矩阵

void main()
{
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
)";

const char* chCheckerboardFS = R"(
#version 330 core
out vec4 fragColor;

uniform float uCellSize   = 40.0;
uniform vec3 uLightColor = vec3(1.0);
uniform vec3 uDarkColor  = vec3(0.8);

void main()
{
    ivec2 board = ivec2(gl_FragCoord.xy / max(uCellSize, 1.0));
    bool black  = (board.x + board.y) % 2 == 0;
    fragColor   = vec4(black ? uDarkColor : uLightColor, 1.0);
}
)";