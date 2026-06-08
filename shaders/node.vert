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

	vec2 screenPos = aOffset + aPos * uRadius / (uResolution * 0.5);
	gl_Position = vec4(screenPos, 0.0, 1.0);
}
// Resolution fix because circles were elliptical
