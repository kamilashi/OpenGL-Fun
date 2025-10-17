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

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;
uniform vec3 uMainLightDirection;
uniform sampler2D uDebugTex;

void main() 
{
    float depthValue = texture(uDebugTex, TexCoord).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}
