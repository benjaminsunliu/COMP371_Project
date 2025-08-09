#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec2 aUV;
                layout (location = 3) in vec3 aNormal;    

                out vec3 vertexColor;
                out vec2 vertexUV;
                out vec3 vertexNormal;
                out vec3 crntPos;

                uniform mat4 camMatrix;
                uniform mat4 model;

                void main()
                {
                    crntPos = vec3(model * vec4(aPos, 1.0f));
                    gl_Position = camMatrix * model * vec4(aPos, 1.0f);

                    vertexColor = aColor;
                    vertexUV = aUV;
                    vertexNormal = aNormal;
                }