#shader vertex
#version 330 core
#include noises.glsl

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
    gl_Position = uProjection * uView * uTransform * vec4(aPos, 1.0);

    WorldPos = vec3(uTransform * vec4(aPos, 1.0));
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

out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;
uniform vec3 uMainLightDirection;
uniform sampler2D depthMap;

void main() 
{
    float depthValue = texture(depthMap, TexCoord).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}