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

vec2 getNoiseGradient(float centerHeight, vec2 uv, float step, float intensity, float baseScale)
{
    float hx  = GradientNoise01(uv + vec2(step, 0), baseScale) * intensity;
    float hz  = GradientNoise01(uv + vec2(0, step), baseScale) * intensity;

    vec2 gradient = getGradient(centerHeight, hx, hz, step, vec2(2.0, 2.0));
    return gradient;
}

float fbmHeight(vec2 sampleCoords, vec3 intensity, vec3 erosionIntensity, float baseScale, float lacunarity)
{
    float sampleScale = baseScale;
    float height1 = GradientNoise01(sampleCoords, sampleScale) * intensity.x;
    height1 = erode(height1, getNoiseGradient(height1, sampleCoords, 0.1, intensity.x, sampleScale), erosionIntensity.x);
    
    sampleScale *= lacunarity;
    float height2 = GradientNoise01(sampleCoords, sampleScale) * intensity.y;
    height2 = erode(height2, getNoiseGradient(height2, sampleCoords, 0.1, intensity.y, sampleScale), erosionIntensity.y);

    sampleScale *= lacunarity;
    float height3 = GradientNoise01(sampleCoords, sampleScale) * intensity.z;
    height3 = erode(height3, getNoiseGradient(height3, sampleCoords, 0.1, intensity.z, sampleScale), erosionIntensity.z);

    float heightOffset = height1 + height2 + height3;

    return heightOffset;
}

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

void main() 
{
    WorldPos = vec3(uTransform * vec4(aPos, 1.0));
#ifndef SHADOW_DEPTH_PASS
    FragPosLightSpace = uLightSpaceMatrix * vec4(WorldPos, 1.0);
#endif
    gl_Position = uProjection * uView * vec4(WorldPos, 1.0);

    TexCoord = aTexCoord;
    Normal = aNormal;
}

