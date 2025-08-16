#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;

void main() 
{
    gl_Position = uProjection * uView * uTransform * vec4(aPos, 1.0f);
    TexCoord = aTexCoord;
}

#shader fragment
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 uMainColor;
uniform vec3 uMainLightColor;

void main() 
{
    FragColor = vec4(uMainColor * uMainLightColor, 1.0f);
}