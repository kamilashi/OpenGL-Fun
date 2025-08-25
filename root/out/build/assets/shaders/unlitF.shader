#version 330 core

out vec4 FragColor;
uniform vec3 uMainColor; 

void main() 
{
    FragColor = vec4(uMainColor, 1.0f);
}
