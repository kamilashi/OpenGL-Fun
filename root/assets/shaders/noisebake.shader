#shader vertex
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

#shader fragment
#version 330 core

#include noises.glsl

in vec2 SampleCoord;
in vec2 TexCoord;

uniform float uLacunarity;
uniform vec3 uAmplitudes;
uniform vec3 uErosionIntensity;

layout(location = 0) out float outNoise;

void main() 
{
    float sampleScale = 3.0;

    float heightOffset = fbmHeight(SampleCoord, uAmplitudes, uErosionIntensity, sampleScale, uLacunarity);

    outNoise = heightOffset;
}