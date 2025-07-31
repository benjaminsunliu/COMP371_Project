#version 330 core
        in vec3 vertexColor;
        in vec3 vertexNormal;
        out vec4 FragColor;
        void main()
        {
           FragColor = vec4(0.5f*vertexNormal+vec3(0.5f), 1.0f);
        };