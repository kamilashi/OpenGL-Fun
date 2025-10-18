#shader vertex
#version 330 core
#include noises.glsl

vec2 getHeightGradient(float centerHeight, vec2 uv, float step, float worldStepScale, sampler2D fbmNoiseTex)
{
    float hx  = texture(fbmNoiseTex, uv + vec2(step, 0)).r;
    float hz  = texture(fbmNoiseTex, uv + vec2(0, step)).r;

    vec2 worldScale = vec2(worldStepScale, worldStepScale); //vec2(1.0, 1.0);
    vec2 gradient = getGradient(centerHeight, hx, hz, step, worldScale);
    return gradient;
}

vec3 normalFromHeight(vec2 gradient) 
{   
    vec3 n = vec3(-gradient.x, 1.0, -gradient.y);
    return normalize(n);
}

vec3 normalFromGradient4D(vec2 uv, float step, float worldStep, sampler2D fbmNoiseTex) 
{   
    float globalAmp = 1;
    vec2 texel = vec2(step); 


    float hL = textureLod(fbmNoiseTex, uv + vec2(-texel.x, 0.0), 0.0).r;
    float hR = textureLod(fbmNoiseTex, uv + vec2( texel.x, 0.0), 0.0).r;
    float hD = textureLod(fbmNoiseTex, uv + vec2(0.0, -texel.y), 0.0).r;
    float hU = textureLod(fbmNoiseTex, uv + vec2(0.0,  texel.y), 0.0).r;

    float dHx = (hR - hL) * 0.5;
    float dHy = (hU - hD) * 0.5;

    vec3 dPosdU = vec3(worldStep,  dHx * globalAmp, 0.0);
    vec3 dPosdV = vec3(0.0,        dHy * globalAmp, worldStep);

    return normalize(cross(dPosdV, dPosdU));
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

uniform sampler2D uFbmNoiseMap;
uniform ivec2 uTextureSize;

uniform float uTime;

void main() 
{
    TexCoord = aTexCoord;
    vec3 localPos = aPos;
    Normal = aNormal;
    
    if(aPos.y > 0.0)
    {
        //TexCoord.x += 0.001;
        float heightOffset = texture(uFbmNoiseMap, TexCoord).r;
        float offsetCompensation = 1.5;
       
        localPos.y += heightOffset - offsetCompensation;
        
        if(localPos.y < -1)
        {
            localPos.y = aPos.y;
        }

        if(dot(aNormal, vec3(0.0, 1.0, 0.0)) > 0.0)
        {
            float step = 1.0 / 257.0;
            float worldStepScale = 1.0; //0.007812; // must correspond to the one use in noisebake pass!

            vec2 gradient = getHeightGradient(heightOffset, TexCoord, step, worldStepScale, uFbmNoiseMap);
            Normal = normalFromHeight(gradient);
            //Normal = normalFromGradient4D(TexCoord, step, worldStepScale, uFbmNoiseMap);
            //Grad = gradient;
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
    //FragColor = vec4(Grad, 0,   1);
#endif
}