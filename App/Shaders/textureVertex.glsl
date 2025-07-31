#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec2 aUV;
                
                uniform mat4 world;
                uniform mat4 view = mat4(1.0);  
                uniform mat4 projection = mat4(1.0);
                uniform float uvScale;           
                out vec3 vertexColor;
                out vec2 vertexUV;
                void main()
                {
                   vertexColor = aColor;
                   mat4 modelViewProjection = projection * view * world;
                   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);
                   vertexUV = aUV * uvScale;    
                };