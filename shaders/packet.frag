#version 330 core

in vec2 uv;
in vec3 color;

out vec4 FragColor;

void main()
{
    float d = length(uv);
    float circle = 1.0 - smoothstep(0.6, 1.0, d);
    if (circle < 0.01) discard;

    // glow effect
    float glow = 1.0 - smoothstep(0.0, 1.0, d);
    FragColor = vec4(color + glow * 0.3, circle);
}