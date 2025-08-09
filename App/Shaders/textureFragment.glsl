#version 330 core

        
        out vec4 FragColor;
        
        in vec3 vertexColor;
        in vec2 vertexUV;
        in vec3 vertexNormal;
        in vec3 crntPos;

        uniform sampler2D textureSampler;
        uniform vec4 lightColor;
        uniform vec3 lightPos;
        uniform vec3 camPos;
        
        void main()
        {
            // ambient lighting
            float ambient = 0.20f;

            // diffuse lighting
            vec3 normal = normalize(vertexNormal);
            vec3 lightDirection = normalize(lightPos - crntPos);
            float diffuse = max(dot(normal, lightDirection), 0.0f);

            // spec lighting
            float specularLight = 0.50f;
            vec3 viewDirection = normalize(camPos - crntPos);
            vec3 reflectDirection = reflect(-lightDirection, normal);
            float specAmount = pow(max(dot(viewDirection, reflectDirection), 0.0f), 8);
            float specular = specAmount * specularLight;

            // final color output
            FragColor = texture(textureSampler, vertexUV) * lightColor * (diffuse + ambient + specular);
            
        }