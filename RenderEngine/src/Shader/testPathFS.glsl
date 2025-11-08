#version 330 core

in vec4 vPosition;
out vec4 fragColor;

void main()
{
    float r = clamp(vPosition.x, 0.0, 1.0);
    float g = clamp(vPosition.y, 0.0, 1.0);
    float b = clamp(r + g, 0.0, 1.0);

    fragColor = vec4(r, g, 0, 1.0);
}