#shader vertex
#version 330 core
#include noises.glsl

vec3 normalFromHeightNoise(vec2 uv, float step, float heightScale, float sampleScale, float hc) 
{
    float hx  = GradientNoise01(uv + vec2(step, 0), sampleScale) * heightScale;
    float hz  = GradientNoise01(uv + vec2(0, step), sampleScale) * heightScale;

    float dhdx_tex = (hx - hc) / step;
    float dhdz_tex = (hz - hc) / step;

    float worldScaleX = 1.0; 
    float worldScaleZ = 1.0; 
    float dhdx = dhdx_tex / (1.0 / worldScaleX); 
    float dhdz = dhdz_tex / (1.0 / worldScaleZ); 

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
    float heightMaxAmplitude = intensity.x + intensity.y + intensity.z;
    float heightOffset = GradientNoise01(sampleCoords, sampleScale) * intensity.x;
    
    heightOffset += GradientNoise01(sampleCoords, sampleScale * 2) * intensity.y;

    heightOffset += GradientNoise01(sampleCoords, sampleScale * 4) * intensity.z;

    vec3 localPos = aPos;
    localPos.y += heightOffset - heightMaxAmplitude/2;

    if(localPos.y < -1)
    {
        localPos.y = aPos.y;
    }

    gl_Position = uProjection * uView * uTransform * vec4(localPos, 1.0);

    if(dot(aNormal, vec3(0.0, 1.0, 0.0)) > 0)
    {
        Normal = -normalFromHeightNoise(sampleCoords, 0.1, heightMaxAmplitude, sampleScale * 2, heightOffset);
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