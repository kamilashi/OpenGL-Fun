#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uTransform;

void main() 
{
    gl_Position = uTransform * vec4(aPos, 1.0f);
}

#shader fragment
#version 330 core

out vec4 FragColor;
uniform vec4 uColor; 

void main() 
{
    FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}