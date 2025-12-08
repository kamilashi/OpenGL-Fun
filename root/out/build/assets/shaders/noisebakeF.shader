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

void getNoiseGradientsLinear2DTexture(vec2 uv, float step, vec3 intensities, float worldStepScale, sampler2D fbmNoiseTex, 
                  out vec2 gradientScale1,
                  out vec2 gradientScale2,
                  out vec2 gradientScale3)
{
    float halfStep = step * 0.5;
    vec3 hx_r  = texture(fbmNoiseTex, uv + vec2(halfStep, 0)).rgb * intensities;
    vec3 hz_r  = texture(fbmNoiseTex, uv + vec2(0, halfStep)).rgb * intensities;

    vec3 hx_l  = texture(fbmNoiseTex, uv + vec2(-halfStep, 0)).rgb * intensities;
    vec3 hz_l  = texture(fbmNoiseTex, uv + vec2(0, -halfStep)).rgb * intensities;

    gradientScale1 = getGradientLinear2D(hx_r.x, hx_l.x, hz_r.x, hz_l.x, step, vec2(worldStepScale, worldStepScale));
    gradientScale2 = getGradientLinear2D(hx_r.y, hx_l.y, hz_r.y, hz_l.y, step, vec2(worldStepScale, worldStepScale));
    gradientScale3 = getGradientLinear2D(hx_r.z, hx_l.z, hz_r.z, hz_l.z, step, vec2(worldStepScale, worldStepScale));
}

float fbmHeightTexture(vec2 sampleCoords, vec3 intensity, vec3 erosionIntensity, float step, float worldStepScale, sampler2D fbmNoiseTex)
{   
    //float step = 1.0 / 256; //0.1;

    vec2 grad1, grad2, grad3;

    getNoiseGradientsLinear2DTexture(sampleCoords, step, intensity, worldStepScale, fbmNoiseTex, grad1, grad2, grad3);
    vec4 noisesTex = texture(fbmNoiseTex, sampleCoords);

    float height1 = noisesTex.r * intensity.x;
    height1 = erode(height1, grad1, erosionIntensity.x);
    
    float height2 = noisesTex.g * intensity.y;
    height2 = erode(height2, grad2, erosionIntensity.y);

    float height3 = noisesTex.b * intensity.z;
    height3 = erode(height3, grad3, erosionIntensity.z);

    float heightOffset = height1 + height2 + height3;

    return heightOffset;
}

vec3 normalFromHeight(vec2 gradient, float step) 
{  
    vec3 n = vec3(gradient.x, 1.0, gradient.y);
    return normalize(n);
}

//in vec2 SampleCoord;
in vec2 TexCoord;

uniform ivec2 uTextureSize;
uniform float uLacunarity;
uniform vec2 uSampleOffset;
uniform float uTime;

layout(location = 0) out vec3 outNoise;

void main() 
{
    //vec2 uv = (gl_FragCoord.xy - 0.5) / uTextureSize;
    float scrollSpeed = 0.27;
    vec2 SampleCoord = TexCoord + vec2(0, uTime * scrollSpeed) + uSampleOffset;

    float sampleScale = 1.0;

    float noise1 = GradientNoise01(SampleCoord, sampleScale);
    sampleScale *= uLacunarity;
    
    float noise2 = GradientNoise01(SampleCoord, sampleScale);
    sampleScale *= uLacunarity;

    float noise3 = GradientNoise01(SampleCoord, sampleScale);

    outNoise = vec3(noise1, noise2, noise3);
}
