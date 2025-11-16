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
    //vec3 n = vec3(-gradient.y, 1.0, -gradient.x); // fixed
    vec3 n = vec3(gradient.x, 1.0, gradient.y); // old
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
        float worldStepScale = 2.0;

        float step = 0.0039;
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

