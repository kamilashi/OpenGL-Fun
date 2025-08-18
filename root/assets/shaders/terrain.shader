#shader vertex
#version 330 core
#include noises.glsl

float fbmHeight(vec2 sampleCoords, vec3 intensity, float baseScale)
{
    float heightOffset = GradientNoise01(sampleCoords, baseScale) * intensity.x;
    
    heightOffset += GradientNoise01(sampleCoords, baseScale * 2) * intensity.y;

    heightOffset += GradientNoise01(sampleCoords, baseScale * 4) * intensity.z;

    return heightOffset;
}

vec3 normalFromHeightNoise(vec2 uv, float step, vec3 intensity, float baseScale, float hc) 
{
    float hx  = fbmHeight(uv + vec2(step, 0), intensity, baseScale);
    float hz  = fbmHeight(uv + vec2(0, step), intensity, baseScale);

    float grad_x = (hx - hc) / step;
    float grad_z = (hz - hc) / step;

    float worldScaleX = 2.0; 
    float worldScaleZ = 2.0; 
    float dhdx = grad_x / worldScaleX; 
    float dhdz = grad_z / worldScaleZ; 

    vec3 n = vec3(-dhdx, 1.0, -dhdz);
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

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;

void main() 
{
    WorldPos = vec3(uTransform * vec4(aPos, 1.0));
    TexCoord = aTexCoord;

    float scrollSpeed = 0.1;
    vec2 sampleCoords = TexCoord + vec2(uTime * scrollSpeed, uTime * -scrollSpeed);

    vec3 intensity = vec3(0.7, 1.5, 1.3);
    float sampleScale = 3.0;
    float heightOffset = fbmHeight(sampleCoords, intensity, sampleScale);

    vec3 localPos = aPos;
    localPos.y += heightOffset - 2.0f;

    if(localPos.y < -1)
    {
        localPos.y = aPos.y;
    }

    gl_Position = uProjection * uView * uTransform * vec4(localPos, 1.0);

    if(dot(aNormal, vec3(0.0, 1.0, 0.0)) > 0)
    {
        Normal = normalFromHeightNoise(sampleCoords, 0.1, intensity, sampleScale, heightOffset);
    }
    else
    {
        Normal = aNormal;
    }
}

#shader fragment
#version 330 core
#include helpers.glsl
#include noises.glsl

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;
uniform vec3 uMainLightDirection;

void main() 
{
    vec3 norm = Normal;

    #include dither.glsl
}