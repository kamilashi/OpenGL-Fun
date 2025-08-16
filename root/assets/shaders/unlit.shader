#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;

void main() 
{
    gl_Position = uProjection * uView * uTransform * vec4(aPos, 1.0f);
}

#shader fragment
#version 330 core

out vec4 FragColor;
uniform vec3 uMainColor; 

void main() 
{
    FragColor = vec4(uMainColor, 1.0f);
}