#version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        layout (location = 2) in vec3 aNormal;
        
        out vec3 vertexColor;
        out vec3 vertexNormal;
        uniform mat4 world;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main()
        {
           vertexNormal = aNormal;
           vertexColor = aColor;
           gl_Position = projection * view * world * vec4(aPos, 1.0);
        };