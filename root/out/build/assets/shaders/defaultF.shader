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

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

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
    vec3 norm = normalize(Normal);


// dither

    float dotPr = dot(norm, uMainLightDirection);

   // float random = GradientNoise01(genSeed(WorldPos), 1000.0); 
    float random = GradientNoise01(gl_FragCoord.xy / vec2(10000, 10000), 5000.0); 
    

    float dotScaled = dotPr * 2;

    float ditherFactor = 1.0;
    // total light                 // total shade
    if(random > (1.0 + dotScaled)  ||  random > (1.0 - dotScaled))
    {
        ditherFactor = 0.0;
    }
    

    float maxBias  = 0.01;   
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
#endif
}
