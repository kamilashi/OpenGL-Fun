#shader fragment
#version 330 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 WorldPos; 

out vec4 FragColor;

void main() 
{
    gl_FragDepth = gl_FragCoord.z;

    //FragColor = vec4(gl_FragDepth, gl_FragDepth, gl_FragDepth, 1.0f);
}