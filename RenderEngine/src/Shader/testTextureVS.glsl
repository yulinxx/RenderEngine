#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(in_pos, 1.0);

    texCoords = in_uv;
}