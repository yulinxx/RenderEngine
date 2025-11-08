
#version 330 core

uniform mat3 oriCamera;
uniform mat3 offsetCamera;

uniform float depth;

in vec2 pos;
in vec2 offset;
in vec2 uv;

out vec2 fragUV;

void main()
{
    vec3 pos3 = vec3(pos, 1.0);
    vec3 offset3 = vec3(offset, 1.0);
    vec3 zeroOffset3 = vec3(0.0, 0.0, 1.0);

    vec3 transformedPos = oriCamera * pos3;
    vec3 transformedOffset = offsetCamera * offset3 - offsetCamera * zeroOffset3;

    transformedPos += transformedOffset;

    gl_Position = vec4(transformedPos.xy, depth, 1.0);
    fragUV = uv;
}
