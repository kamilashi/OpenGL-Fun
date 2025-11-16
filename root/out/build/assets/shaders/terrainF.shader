#version 330 core

uint hash_u32(uint x) 
{         
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

uint hash_u32(uvec2 v) 
{ 
    return hash_u32(v.x ^ (0x9e3779b9u + (v.y<<6) + (v.y>>2)));
}

float rand01(uvec2 seed) 
{
    return float(hash_u32(seed)) * (1.0 / 4294967296.0); 
}

vec2 genSeed(vec3 vertexPos, vec2 fraqUV)
{
    vec2 seed = vec2((vertexPos.x * vertexPos.y / (fraqUV.x) * 50.0 + vertexPos.z), (vertexPos.z * vertexPos.y / (fraqUV.y) * 50.0 + vertexPos.x));

    return seed;
}

vec2 genSeed(vec3 vertexPos)
{
    vec2 seed = vec2((vertexPos.x * vertexPos.y + vertexPos.z), (vertexPos.z * vertexPos.y + vertexPos.x));

    return seed;
}

vec2 genSeed(vec2 vertexPos, vec2 fraqUV)
{
    vec2 seed = vec2((vertexPos.x / (fraqUV.x) * 50.0), (vertexPos.y / (fraqUV.y) * 50.0));

    return seed;
}



vec2 getShadow(vec4 fragPosLightSpace, sampler2D shadowMap, float shadowBias)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float shadow = currentDepth - shadowBias > closestDepth  ? 1.0 : 0.0;  

    return vec2(shadow, currentDepth);
}  
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

float getGradientLinear(float fr, float fl, float step, float worldScale)
{
    float grad = (fr - fl) / step;
    
    grad = grad / worldScale; 

    return grad;
}

vec2 getGradientLinear2D(float xr, float xl, float zr, float zl, float step, vec2 worldScale)
{
    float grad_x = getGradientLinear(xr, xl, step, worldScale.x);
    float grad_z = getGradientLinear(zr, zl, step, worldScale.y);
    
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

vec2 getNoiseGradientLinear2D(vec2 uv, float step, float intensity, float baseScale, float worldStepScale)
{
    float halfStep = step * 0.5;
    float hx_r  = GradientNoise01(uv + vec2(halfStep, 0), baseScale) * intensity;
    float hz_r  = GradientNoise01(uv + vec2(0, halfStep), baseScale) * intensity;

    float hx_l  = GradientNoise01(uv + vec2(-halfStep, 0), baseScale) * intensity;
    float hz_l  = GradientNoise01(uv + vec2(0, -halfStep), baseScale) * intensity;

   // vec2 gradient = getGradient(centerHeight, hx, hz, step, vec2(worldStepScale, worldStepScale));
    vec2 gradient = getGradientLinear2D(hx_r, hx_l, hz_r, hz_l, step, vec2(worldStepScale, worldStepScale));
    return gradient;
}

float fbmHeight(vec2 sampleCoords, vec3 intensity, vec3 erosionIntensity, float baseScale, float lacunarity, float step, float worldStepScale)
{   
    //float step = 1.0 / 256; //0.1;
    float sampleScale = baseScale;
    float height1 = GradientNoise01(sampleCoords, sampleScale) * intensity.x;
    //height1 = erode(height1, getNoiseGradient(height1, sampleCoords, step, intensity.x, sampleScale, worldStepScale), erosionIntensity.x);
    height1 = erode(height1, getNoiseGradientLinear2D(sampleCoords, step, intensity.x, sampleScale, worldStepScale), erosionIntensity.x);
    
    sampleScale *= lacunarity;
    float height2 = GradientNoise01(sampleCoords, sampleScale) * intensity.y;
    height2 = erode(height2, getNoiseGradientLinear2D(sampleCoords, step, intensity.y, sampleScale, worldStepScale), erosionIntensity.y);

    sampleScale *= lacunarity;
    float height3 = GradientNoise01(sampleCoords, sampleScale) * intensity.z;
    height3 = erode(height3, getNoiseGradientLinear2D(sampleCoords, step, intensity.z, sampleScale, worldStepScale), erosionIntensity.z);

    float heightOffset = height1 + height2 + height3;

    return heightOffset;
}

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

// dither

    float dotPr = dot(norm, uMainLightDirection);

   // float random = GradientNoise01(genSeed(WorldPos), 1000.0); 
    float random = GradientNoise01(gl_FragCoord.xy / vec2(10000, 10000), 5000.0); 
    

    float dotScaled = dotPr * 2;

    float ditherFactor = 1.0;
    // total light                 // total shade
    if(random > (1.0 + dotScaled) ||  random > (1.0 - dotScaled))
    {
        ditherFactor = 0.0;
    }
    

    float maxBias  = 0.008;   
    float slopeScale = 0.01;    

    float bias = max(slopeScale * (1.0 - max(-dotPr, 0)), maxBias);
    vec2 castShadowAndDepth = getShadow(FragPosLightSpace, shadowMap, bias); 
    
    float castShadow = castShadowAndDepth.x;
    float depth = 0.0; // castShadowAndDepth.y;

    vec3 ditherColor = vec3(0.0, 0.00, 0.0); // vec3(0.3, 0.05, 0.1);//
    vec3 color = uMainLightColor;

    if(dotScaled > 0.0 || castShadow > 0) // shaded areas
    {
        vec3 ambientColor = mix(vec3(0.27, 0.3, 0.8), vec3(0.3, 0.5, 1.0), (max(dotScaled, 0.0)));
        color = ambientColor;
    }
    else  // lit areas
    {
        vec3 colorBlend = mix(uMainLightColor, uMainLightColor + vec3(0.1, 0.1, 0.1), (max(-dotScaled, 0.0)));
        color = colorBlend;
    }

    vec3 diffuse = mix(color, ditherColor, ditherFactor);

    FragColor = vec4( (diffuse * (1 - depth)) * uMainColor, 1.0);
    //FragColor = vec4( norm, 1.0);
    //FragColor = vec4(Grad, 0,   1);
#endif
}
