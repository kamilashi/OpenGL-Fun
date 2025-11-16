#shader vertex
#version 330 core
#include noises.glsl

vec2 getHeightGradient(float centerHeight, vec2 uv, float step, vec3 intensity, vec3 erosionIntensity, float baseScale, float lacunarity, float worldStepScale)
{
    float halfStep = step * 0.5;
    float hx_r  = fbmHeight(uv + vec2(halfStep, 0), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);
    float hz_r  = fbmHeight(uv + vec2(0, halfStep), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);

    float hx_l = fbmHeight(uv + vec2(-halfStep, 0), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);
    float hz_l  = fbmHeight(uv + vec2(0, -halfStep), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);

    //vec2 worldScale = vec2(2.0, 2.0);
    vec2 worldScale = vec2(worldStepScale, worldStepScale);
    vec2 gradient = getGradientLinear2D(hx_r, hx_l, hz_r, hz_l, step, worldScale);
    return gradient;
}

vec2 getHeightGradientFromCenter(float centerHeight, vec2 uv, float step, vec3 intensity, vec3 erosionIntensity, float baseScale, float lacunarity, float worldStepScale)
{
    float hx  = fbmHeight(uv + vec2(step, 0), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);
    float hz  = fbmHeight(uv + vec2(0, step), intensity, erosionIntensity, baseScale, lacunarity, step, worldStepScale);

    //vec2 worldScale = vec2(2.0, 2.0);
    vec2 worldScale = vec2(worldStepScale, worldStepScale);
    vec2 gradient = getGradient(centerHeight, hx, hz, step, worldScale);
    return gradient;
}

vec3 normalFromHeight(vec2 gradient, float step) 
{   
    vec3 n = vec3(gradient.x, 1.0, gradient.y);
    return normalize(n);
}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 WorldPos; 

#ifndef SHADOW_DEPTH_PASS
    out vec4 FragPosLightSpace;
    uniform mat4 uLightSpaceMatrix;
#endif

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;

uniform float uLacunarity;
uniform vec3 uAmplitudes;
uniform vec3 uErosionIntensity;
uniform vec2 uSampleOffset;

uniform float uTime;

void main() 
{
    TexCoord = aTexCoord;
    vec3 localPos = aPos;
    Normal = aNormal;
    
    if(aPos.y > 0.0)
    {
        float scrollSpeed = 0.27;
        vec2 sampleCoords = TexCoord + vec2(0, uTime * scrollSpeed) + uSampleOffset;

        float sampleScale = 1.0;
        float worldStepScale = 1.0;

        float step = 0.01;//0.0039;
        float heightOffset = fbmHeight(sampleCoords, uAmplitudes, uErosionIntensity, sampleScale, uLacunarity, step, worldStepScale);
        float offsetCompensation = 1.5;
       
        localPos.y += heightOffset - offsetCompensation;
        
        if(localPos.y < -1)
        {
            localPos.y = aPos.y;
        }

        if(aNormal.y > 0.0)
        {
            vec2 gradient = getHeightGradient(heightOffset, sampleCoords, step, uAmplitudes, uErosionIntensity, sampleScale, uLacunarity, worldStepScale);
            Normal = normalFromHeight(gradient, step);
        }
    }

    WorldPos = vec3(uTransform * vec4(localPos, 1.0));
#ifndef SHADOW_DEPTH_PASS
    FragPosLightSpace = uLightSpaceMatrix * vec4(WorldPos, 1.0);
#endif
    gl_Position = uProjection * uView * vec4(WorldPos, 1.0);
}

#shader fragment
#version 330 core
#include helpers.glsl
#include noises.glsl

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

#ifndef SHADOW_DEPTH_PASS
    in vec4 FragPosLightSpace;

    uniform sampler2D shadowMap;
    
    uniform vec3 uMainColor;
    uniform vec3 uMainLightColor;
    uniform vec3 uMainLightDirection;

    uniform float uTime;
#endif

out vec4 FragColor;

void main() 
{
#ifndef SHADOW_DEPTH_PASS
    vec3 norm = Normal;
    #include dither.glsl
#endif
}