#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

//out vec2 SampleCoord;
out vec2 TexCoord;

void main() 
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}

#shader fragment
#version 330 core

#include noises.glsl

//in vec2 SampleCoord;
in vec2 TexCoord;

uniform ivec2 uTextureSize;
uniform float uLacunarity;
uniform vec3 uAmplitudes;
uniform vec3 uErosionIntensity;

uniform vec2 uSampleOffset;
uniform float uTime;

layout(location = 0) out float outNoise;

void main() 
{
    //vec2 uv = (gl_FragCoord.xy - 0.5) / uTextureSize;
    float scrollSpeed = 0.27;
    vec2 SampleCoord = TexCoord + vec2(0, uTime * scrollSpeed) + uSampleOffset;

    float sampleScale = 1.0;
    float worldStepScale = 1.0;
    float step =  1.0 / 267.0;

    float heightOffset = fbmHeight(SampleCoord, uAmplitudes, uErosionIntensity, sampleScale, uLacunarity, step, worldStepScale);

    outNoise = heightOffset;
}