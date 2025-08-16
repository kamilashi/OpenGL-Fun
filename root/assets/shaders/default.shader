#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 WorldPos; 

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;

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

    vec3 lightPos = vec3(2.0, 2.0, -2.0);
    vec3 lightDir = normalize( lightPos - WorldPos);
    float dot = (dot(norm, lightDir));

   uvec2 seed = uvec2(abs(WorldPos.x * WorldPos.y / TexCoord.x * 50.0f + WorldPos.z), abs(WorldPos.z * WorldPos.y / TexCoord.y * 50.0f + WorldPos.x));
    
    float random = rand01(seed);
    float multiplier = 0.0;
    if(random > (1.0 + dot))
    {
        multiplier = 1.0;
    }

    vec3 diffuse = multiplier * uMainColor;
    float normalizedDot = 0.5*(dot + 1);

    FragColor = vec4( max(dot, 0) * uMainLightColor, 1.0);
}