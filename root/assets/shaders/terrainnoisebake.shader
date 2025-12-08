#shader vertex
#version 330 core
#include noises.glsl
#include helpers.glsl

vec2 getHeightGradient(float centerHeight, vec2 uv, float step, vec3 intensity, vec3 erosionIntensity, float worldStepScale, sampler2D fbmNoiseTex)
{
    float halfStep = step * 0.5;
    float hx_r = fbmHeightTexture(uv + vec2(halfStep, 0), intensity, erosionIntensity, step, worldStepScale, fbmNoiseTex);
    float hz_r = fbmHeightTexture(uv + vec2(0, halfStep), intensity, erosionIntensity, step, worldStepScale, fbmNoiseTex);

    float hx_l = fbmHeightTexture(uv + vec2(-halfStep, 0), intensity, erosionIntensity, step, worldStepScale, fbmNoiseTex);
    float hz_l = fbmHeightTexture(uv + vec2(0, -halfStep), intensity, erosionIntensity, step, worldStepScale, fbmNoiseTex);

    //vec2 worldScale = vec2(2.0, 2.0);
    vec2 worldScale = vec2(worldStepScale, worldStepScale);
    vec2 gradient = getGradientLinear2D(hx_r, hx_l, hz_r, hz_l, step, worldScale);
    return gradient;
}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 WorldPos; 
out vec2 Grad; 

#ifndef SHADOW_DEPTH_PASS
    out vec4 FragPosLightSpace;
    uniform mat4 uLightSpaceMatrix;
#endif

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uAmplitudes;
uniform vec3 uErosionIntensity;
uniform sampler2D uFbmNoiseMap;
uniform ivec2 uTextureSize;

void main() 
{
    float step = 1.0 / 257; //0.01; //0.0039;

    TexCoord = remapUV(aTexCoord, vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(step, step), vec2(1.0 - step, 1.0 - step));
    vec3 localPos = aPos;
    Normal = aNormal;
    
     if(aPos.y > 0.0)
    {
        vec2 sampleCoords = TexCoord;

        float worldStepScale = 1.0; //0.0039;

        float heightOffset = fbmHeightTexture(sampleCoords, uAmplitudes, uErosionIntensity, step, worldStepScale, uFbmNoiseMap);
        float offsetCompensation = 1.5;
       
        localPos.y += heightOffset - offsetCompensation;
        
        if(localPos.y < -1)
        {
            localPos.y = aPos.y;
        }

        if(dot(aNormal, vec3(0.0, 1.0, 0.0)) > 0.0)
        {
            vec2 gradient = getHeightGradient(heightOffset, sampleCoords, step, uAmplitudes, uErosionIntensity, worldStepScale, uFbmNoiseMap);
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
in vec2 Grad; 

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
    //FragColor = vec4(norm,   1);
#endif
}