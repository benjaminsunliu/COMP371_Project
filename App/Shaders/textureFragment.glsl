#version 330 core
        in vec3 vertexColor;
        in vec2 vertexUV;
        uniform sampler2D textureSampler;
        uniform bool useBlackKey;
        out vec4 FragColor;
        void main()
        {
            vec4 tex = texture(textureSampler, vertexUV);
            if (useBlackKey) {
                if (tex.r < 0.05 && tex.g < 0.05 && tex.b < 0.05) discard;
                FragColor = vec4(tex.rgb, 1.0);
            } else {
                FragColor = tex;
            }
        }