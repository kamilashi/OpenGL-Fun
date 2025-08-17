#shader vertex
#version 330 core
#include noises.glsl

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

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

    
    float dotUp = dot(aNormal, vec3(0.0, 1.0, 0.0));
    if(dotUp >= 0.0)
    {
        gl_Position.y += GradientNoise01(aTexCoord, 1.0) * 5.0;
    }
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

void main() 
{
    vec3 norm = normalize(Normal);

    float dot = dot(norm, uMainLightDirection);

    vec2 seed = genSeed(WorldPos, TexCoord);
    
    float random = GradientNoise01(seed, 10.0); 
    float multiplier = 0.0;
    if(random > (1.0 + dot))
    {
        multiplier = 1.0;
    }

    vec3 diffuse = multiplier * uMainColor;

    FragColor = vec4( diffuse * uMainLightColor, 1.0);
}