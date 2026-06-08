#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec2 uResolution;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}