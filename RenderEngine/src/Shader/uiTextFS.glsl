#version 330 core

uniform vec4 color;
uniform sampler2D texSampler;

in vec2 fragUV;

out vec4 fragColor;

void main()
{
    float a = texture(texSampler, fragUV).r; 
    fragColor = vec4(color.rgb, a);
}
