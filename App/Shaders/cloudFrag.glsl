#version 330 core

in vec3 vertexColor;
in vec2 vertexUV;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, vertexUV);

    // Multiply texture color by vertex color (for tinting)
    vec4 color = texColor * vec4(vertexColor, 1.0);

    // Discard fully transparent pixels (for alpha testing, optional)
    if (color.a < 0.1)
        discard;

    FragColor = color;
}
