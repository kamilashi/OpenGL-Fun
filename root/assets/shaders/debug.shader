#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 WorldPos; 

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;

void main() 
{

#ifndef RENDER_IN_SCREEN_SPACE
    gl_Position = uProjection * uView * uTransform * vec4(aPos, 1.0);
    WorldPos = vec3(uTransform * vec4(aPos, 1.0));
#else
    gl_Position = vec4(aPos, 1.0);
#endif

    TexCoord = aTexCoord;
    Normal = aNormal;
}

#shader fragment
#version 330 core
#include helpers.glsl

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 


out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;
uniform vec3 uMainLightDirection;

uniform ivec3 uDebugTexChannelMask; 
uniform sampler2D uDebugTex;

void main() 
{
    vec4 textureSample = texture(uDebugTex, TexCoord);
    float depthValue1 = textureSample.r * uDebugTexChannelMask.x;
    float depthValue2 = textureSample.g * uDebugTexChannelMask.y;
    float depthValue3 = textureSample.b * uDebugTexChannelMask.z;

    FragColor = vec4(depthValue1, depthValue2, depthValue3, 1.0);
}