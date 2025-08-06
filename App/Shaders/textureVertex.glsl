#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal; 

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform float uvScale;

out vec3 vertexColor;
out vec2 vertexUV;
out vec3 fragment_position;
out vec3 fragment_normal;
out vec4 fragment_position_light_space;

void main()
{
    // Compute transformed vertex position
    vec4 worldPosition = world * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosition;

    // Output to fragment shader
    vertexColor = aColor;
    vertexUV = aUV * uvScale;
    fragment_position = worldPosition.xyz;
    fragment_normal = mat3(transpose(inverse(world))) * aNormal; // Properly transform normal
    fragment_position_light_space = lightSpaceMatrix * worldPosition;
}
