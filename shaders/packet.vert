#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aOffset;
layout (location = 2) in vec3 aColor;

out vec2 uv;
out vec3 color;

uniform float uRadius;
uniform vec2 uResolution;

void main()
{
    uv = aPos;
    color = aColor;

    vec2 center = aOffset * uResolution * 0.5;
    vec2 pos = (center + aPos * uRadius) / (uResolution * 0.5);

    gl_Position = vec4(pos, 0.0, 1.0);
}