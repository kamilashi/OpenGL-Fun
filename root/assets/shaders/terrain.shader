#shader vertex
#version 330 core
#include noises.glsl

float erode(float height, vec2 grad, float erosionStrength)
{
    float erodedHeight = height / (1 + length(grad) * erosionStrength);
    return erodedHeight;
}

vec2 getGradient(float fc, float fx, float fz, float step, vec2 worldScale)
{
    float grad_x = (fx - fc) / step;
    float grad_z = (fz - fc) / step;
    
    grad_x = grad_x / worldScale.x; 
    grad_z = grad_z / worldScale.y; 
    
    vec2 gradient = vec2(grad_x, grad_z);
    return gradient;
}

vec2 getNoiseGradient(float centerHeight, vec2 uv, float step, float intensity, float baseScale)
{
    float hx  = GradientNoise01(uv + vec2(step, 0), baseScale) * intensity;
    float hz  = GradientNoise01(uv + vec2(0, step), baseScale) * intensity;

    vec2 gradient = getGradient(centerHeight, hx, hz, step, vec2(2.0, 2.0));
    return gradient;
}

float fbmHeight(vec2 sampleCoords, vec3 intensity, vec3 erosionIntensity, float baseScale)
{
    float sampleScale = baseScale;
    float height1 = GradientNoise01(sampleCoords, sampleScale) * intensity.x;
    height1 = erode(height1, getNoiseGradient(height1, sampleCoords, 0.1, intensity.x, sampleScale), erosionIntensity.x);
    
    sampleScale *= 2;
    float height2 = GradientNoise01(sampleCoords, sampleScale) * intensity.y;
    height2 = erode(height2, getNoiseGradient(height2, sampleCoords, 0.1, intensity.y, sampleScale), erosionIntensity.y);

    sampleScale *= 2;
    float height3 = GradientNoise01(sampleCoords, sampleScale) * intensity.z;
    height3 = erode(height3, getNoiseGradient(height3, sampleCoords, 0.1, intensity.z, sampleScale), erosionIntensity.y);

    float heightOffset = height1 + height2 + height3;

    return heightOffset;
}

vec2 getHeightGradient(float centerHeight, vec2 uv, float step, vec3 intensity, vec3 erosionIntensity, float baseScale)
{
    float hx  = fbmHeight(uv + vec2(step, 0), intensity, erosionIntensity, baseScale);
    float hz  = fbmHeight(uv + vec2(0, step), intensity, erosionIntensity, baseScale);

    vec2 gradient = getGradient(centerHeight, hx, hz, step, vec2(2.0, 2.0));
    return gradient;
}

vec3 normalFromHeight(vec2 gradient) 
{   
    vec3 n = vec3(-gradient.x, 1.0, -gradient.y);
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
uniform float uTime;


void main() 
{
    TexCoord = aTexCoord;

    float scrollSpeed = 0.1;
    vec2 sampleCoords = TexCoord + vec2(uTime * scrollSpeed, uTime * -scrollSpeed);

    vec3 intensity = vec3(1.0, 1.8, 0.5);
    vec3 erosionIntensity = vec3(0.1, 0.3, 1.5);

    float sampleScale = 3.0;

    float heightOffset = fbmHeight(sampleCoords, intensity, erosionIntensity, sampleScale);

    vec2 gradient = getHeightGradient(heightOffset, sampleCoords, 0.01, intensity, erosionIntensity, sampleScale);
    
    if(dot(aNormal, vec3(0.0, 1.0, 0.0)) > 0)
    {
        Normal = normalFromHeight(gradient);
    }
    else
    {
        Normal = aNormal;
    }

    vec3 localPos = aPos;
    float offsetCompensation = 1.5;
    localPos.y += heightOffset - offsetCompensation;

    if(localPos.y < -1)
    {
        localPos.y = aPos.y;
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
#endif

out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;
uniform vec3 uMainLightDirection;


void main() 
{
    vec3 norm = Normal;

    #include dither.glsl
}