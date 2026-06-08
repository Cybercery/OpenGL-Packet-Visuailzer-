#version 330 core

in vec2 uv;
in vec3 color;

out vec4 FragColor;

void main()
{
    float d = length(uv);
    float circle = 1.0 - smoothstep(0.85, 1.0, d);
    if (circle < 0.01) discard;
    FragColor = vec4(color, circle);
}