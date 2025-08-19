#shader vertex
#version 330 core
#include noises.glsl
#include helpers.glsl

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
    out float DitherUVSample;
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

#shader fragment
#version 330 core
#include helpers.glsl
#include noises.glsl

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

#ifndef SHADOW_DEPTH_PASS
    in vec4 FragPosLightSpace;
    in float DitherUVSample;
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

    #include dither.glsl
#endif
}