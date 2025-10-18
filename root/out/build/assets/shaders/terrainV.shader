#version 330 core
//Unity's implementation of gradient noise, is here for testing and will be removed later.

vec2 gradientNoiseDir(vec2 p)
{
    p = mod(p, 289.0);
    float x = mod((34.0 * p.x + 1.0) * p.x, 289.0) + p.y;
    x = mod((34.0 * x + 1.0) * x, 289.0);
    x = fract(x / 41.0) * 2.0 - 1.0;
    return normalize(vec2(x - floor(x + 0.5), abs(x) - 0.5));
}

// 2D gradient noise in ~[-1, 1]
float gradientNoise(vec2 p)
{
    vec2 ip = floor(p);
    vec2 fp = fract(p);

    float d00 = dot(gradientNoiseDir(ip),                     fp);
    float d01 = dot(gradientNoiseDir(ip + vec2(0.0, 1.0)),    fp - vec2(0.0, 1.0));
    float d10 = dot(gradientNoiseDir(ip + vec2(1.0, 0.0)),    fp - vec2(1.0, 0.0));
    float d11 = dot(gradientNoiseDir(ip + vec2(1.0, 1.0)),    fp - vec2(1.0, 1.0));

    // quintic fade (Perlin)
    vec2 w = fp * fp * fp * (fp * (fp * 6.0 - 15.0) + 10.0);

    // bilinear interpolation with smooth weights
    return mix( mix(d00, d01, w.y), mix(d10, d11, w.y), w.x );
}

float GradientNoise01(vec2 UV, float Scale)
{
    return 0.5 + 0.5 * gradientNoise(UV * Scale);
}

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

vec2 getNoiseGradient(float centerHeight, vec2 uv, float step, float intensity, float baseScale, float worldStepScale)
{
    float hx  = GradientNoise01(uv + vec2(step, 0), baseScale) * intensity;
    float hz  = GradientNoise01(uv + vec2(0, step), baseScale) * intensity;

    vec2 gradient = getGradient(centerHeight, hx, hz, step, vec2(worldStepScale, worldStepScale));
    return gradient;
}

float fbmHeight(vec2 sampleCoords, vec3 intensity, vec3 erosionIntensity, float baseScale, float lacunarity, float step, float worldStepScale)
{   
    //float step = 1.0 / 267; //0.1;
    float sampleScale = baseScale;
    float height1 = GradientNoise01(sampleCoords, sampleScale) * intensity.x;
    height1 = erode(height1, getNoiseGradient(height1, sampleCoords, step, intensity.x, sampleScale, worldStepScale), erosionIntensity.x);
    
    sampleScale *= lacunarity;
    float height2 = GradientNoise01(sampleCoords, sampleScale) * intensity.y;
    height2 = erode(height2, getNoiseGradient(height2, sampleCoords, step, intensity.y, sampleScale, worldStepScale), erosionIntensity.y);

    sampleScale *= lacunarity;
    float height3 = GradientNoise01(sampleCoords, sampleScale) * intensity.z;
    height3 = erode(height3, getNoiseGradient(height3, sampleCoords, step, intensity.z, sampleScale, worldStepScale), erosionIntensity.z);

    float heightOffset = height1 + height2 + height3;

    return heightOffset;
}

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

