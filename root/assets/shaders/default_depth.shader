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
    WorldPos = vec3(uTransform * vec4(aPos, 1.0));
    gl_Position = uProjection * uView * vec4(WorldPos, 1.0);
}

#shader fragment
#version 330 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

out vec4 FragColor;

void main() 
{
    gl_FragDepth = gl_FragCoord.z;
}