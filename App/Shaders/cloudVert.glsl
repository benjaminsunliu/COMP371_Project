#version 330 core

layout(location = 0) in vec3 aPos;   
layout(location = 1) in vec3 aColor; 
layout(location = 2) in vec2 aUV;    

out vec3 vertexColor;
out vec2 vertexUV;

uniform mat4 model;
uniform mat4 camMatrix;

void main()
{
    vertexColor = aColor;
    vertexUV = aUV;

    // Transform vertex position with model and camera matrices
    gl_Position = camMatrix * model * vec4(aPos, 1.0);
}
