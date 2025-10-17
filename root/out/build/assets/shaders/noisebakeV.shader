#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 SampleCoord;
out vec2 TexCoord;

uniform vec2 uSampleOffset;
uniform float uTime;

void main() 
{
    gl_Position = vec4(aPos, 1.0);

    TexCoord = aTexCoord;

    float scrollSpeed = 0.1;
    SampleCoord = TexCoord + vec2(uTime * scrollSpeed, uTime * -scrollSpeed) + uSampleOffset;
}

