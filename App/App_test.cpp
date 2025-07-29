#include <iostream>
#include "vertexData.h"
#include "CarVertex/CyberTruck.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices

#include <cassert>
#include <glm/common.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace glm;
using namespace std;   

// Mouse state
double lastX = 0.0f, lastY = 0.0f;
float yaw = 0.0f;
bool firstMouse = true;

const char* getVertexShaderSource()
{
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;"
        "layout (location = 1) in vec3 aColor;"
        "layout (location = 2) in vec3 aNormal;"
        ""
        "out vec3 vertexColor;"
        "out vec3 vertexNormal;"
        "uniform mat4 world;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        ""
        "void main()\n"
        "{\n"
        "   vertexNormal = aNormal;\n"
        "   vertexColor = aColor;\n"
        "   gl_Position = projection * view * world * vec4(aPos, 1.0);\n"
        "}\n";
}


const char* getFragmentShaderSource()
{
    return
        "#version 330 core\n"
        "in vec3 vertexColor;"
        "in vec3 vertexNormal;"
        "out vec4 FragColor;"
        "void main()"
        "{"
        "   FragColor = vec4(0.5f*vertexNormal+vec3(0.5f), 1.0f);"
        "}";
}

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource)
{
         // vertex shader
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
}
        // link shaders
        int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;

}

// Texture methods

const char* getTexturedVertexShaderSource()
{
    return
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;"
                "layout (location = 1) in vec3 aColor;"
                "layout (location = 2) in vec2 aUV;"
                ""
                "uniform mat4 world;"
                "uniform mat4 view = mat4(1.0);"  // default value for view matrix (identity)
                "uniform mat4 projection = mat4(1.0);"
                "uniform float uvScale;"           // NEW: scale/tile UVs
                ""
                "out vec3 vertexColor;"
                "out vec2 vertexUV;"
                "void main()"
                "{"
                "   vertexColor = aColor;"
                "   mat4 modelViewProjection = projection * view * world;"
                "   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
                "   vertexUV = aUV * uvScale;"    // NEW: apply scaling
                "}";
}

const char* getTexturedFragmentShaderSource()
{
    return
                "#version 330 core\n"
                "in vec3 vertexColor;"
                "in vec2 vertexUV;"
                "uniform sampler2D textureSampler;"
                ""
                "out vec4 FragColor;"
                "void main()"
                "{"
                "   vec4 textureColor = texture(textureSampler, vertexUV);"
                "   FragColor = textureColor;"
                "}";
}

GLuint loadTexture(const char *filename)
{
    // Step 1 load Textures with dimension data
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data){
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    // Step 2 create and bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Step 3 set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Step 4 upload texture data to GPU
    GLenum format = 0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    std::cout << "Texture " << filename << " channels: " << nrChannels << std::endl;

    // Step 5 Free resources
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    return textureId;
}

// Create a Vertex Array Object (VAO) and Vertex Buffer Object (VBO) for the vertices
GLuint createVAO(float* vertices, size_t size) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return VAO;
}

GLuint createTexturedVAO(float* vertices, size_t size) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);              
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // uv attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    return VAO;
}

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <GL/glew.h>

// Structure to return both VAO and index count
struct ModelData {
    GLuint VAO;
    GLsizei indexCount;
};

ModelData loadModelWithAssimp(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        throw std::runtime_error("Failed to load model: " + path);
    }

    const aiMesh* mesh = scene->mMeshes[0];

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D pos = mesh->mVertices[i];
        aiVector3D normal = mesh->mNormals[i];

        // Position
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        // Default color (white)
        vertices.push_back(1.0f); // R
        vertices.push_back(1.0f); // G
        vertices.push_back(1.0f); // B

        // Texture coordinates (UV)
        if (mesh->HasTextureCoords(0)) {
            aiVector3D uv = mesh->mTextureCoords[0][i];
            vertices.push_back(uv.x);
            vertices.push_back(uv.y);
        } else {
            // No UVs? Use zero
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Load indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set attribute pointers based on this vertex layout:
    // 3 floats position, 3 floats color, 2 floats UV => stride = 8 floats

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return { VAO, static_cast<GLsizei>(indices.size()) };
}






// Set the projection, view, and world matrices in the shader methods
void setProjectionMatrix(int shaderProgram, const glm::mat4& projectionMatrix)
{
    GLuint location = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, const glm::mat4& viewMatrix)
{
    GLuint location = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(location, 1, GL_FALSE, &viewMatrix[0][0]);
}

void setWorldMatrix(int shaderProgram, const glm::mat4& worldMatrix)
{
    GLuint location = glGetUniformLocation(shaderProgram, "world");
    glUniformMatrix4fv(location, 1, GL_FALSE, &worldMatrix[0][0]);
}

int main(int argc, char*argv[])
{
    // Initialize GLFW and OpenGL version
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Allow window resize

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Project", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
    });
    
    // Load textures
    GLuint grassTextureID = loadTexture("Textures/grass.jpg");
    GLuint asphaltTextureID = loadTexture("Textures/asphalt.jpg");
    GLuint curbTextureID = loadTexture("Textures/curb.jpg");
    GLuint cobblestoneTextureID = loadTexture("Textures/cobblestone.jpg");
    GLuint mountainTextureID = loadTexture("Textures/moutain.jpg"); // rock texture for hills
    GLuint lightPoleTextureID = loadTexture("Textures/Light Pole.png");
    

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Black background
    glClearColor(135.0f/255.0f, 206.0f/255.0f, 235.0f/255.0f, 1.0f);
    
    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders(getVertexShaderSource(), getFragmentShaderSource());
    int texturedShaderProgram = compileAndLinkShaders(getTexturedVertexShaderSource(), getTexturedFragmentShaderSource());
    

    glUseProgram(shaderProgram); // Use our shader program


    // Define and upload geometry to the GPU here ...
    GLuint cubeVAO = createVAO(cubeVertices, sizeof(cubeVertices));
    GLuint floorVAO = createTexturedVAO(floorVertices,sizeof(floorVertices));
    GLuint roadVAO = createTexturedVAO(roadVertices, sizeof(roadVertices));
    GLuint curbVAO = createTexturedVAO(const_cast<float*>(curbVerts),sizeof(curbVerts));

    // Camera variables
    glm::vec3 cameraPos   = glm::vec3(0.0f, 1.5f,  5.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    float cameraHorizontalAngle = 270.0f;
    float cameraVerticalAngle = 0.0f;
    bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f;

    // Set up projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f/600.f, 0.1f, 100.0f);
    
    // Set up view matrix
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    GLint modelLocation = glGetUniformLocation(shaderProgram, "world");
    GLint viewLocation  = glGetUniformLocation(shaderProgram, "view");
    GLint projLocation  = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
    
    // disable cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering
    // glEnable(GL_CULL_FACE); This takes off the ability to see the car through the windshield so disabled for now

    // load models
    ModelData cybertruckData = loadModelWithAssimp("Models/SUV.obj");
    ModelData birdData = loadModelWithAssimp("Models/Bird.obj");
    ModelData hillData = loadModelWithAssimp("Models/part.obj");
    ModelData lightPoleData = loadModelWithAssimp("Models/Light Pole.obj");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Frame time calculation
        float deltaTime = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

        // Get the location of the color uniform
        int colorLocation = glGetUniformLocation(shaderProgram, "vertexColor");


        glUseProgram(texturedShaderProgram);
        GLuint textureSamplerLocation = glGetUniformLocation(texturedShaderProgram, "textureSampler");
        GLint uvScaleLocation = glGetUniformLocation(texturedShaderProgram, "uvScale");
        glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, grassTextureID); // Bind the grass texture
        glUniform1i(textureSamplerLocation, 0); // Set the texture sampler to use texture unit 0
        glUniform1f(uvScaleLocation, 1.0f); // floor UV scale
       
        // Draw the floor
        glm::mat4 floorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.01f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.02f, 10.0f));
        setWorldMatrix(texturedShaderProgram, floorModel);
        setProjectionMatrix(texturedShaderProgram, projection);
        setViewMatrix(texturedShaderProgram, view);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Draw the hills with rock texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mountainTextureID);
        glUniform1i(textureSamplerLocation, 0);
        for (int i = 0; i < 24; ++i) {
            glm::vec3 hillPosition;
            float hillScale = 0.5f;
            if (i == 0) {
                hillPosition = glm::vec3(-15.0f, 0.0f, -7.0f);
                hillScale = 0.5f;
            } else if (i == 1) {
                hillPosition = glm::vec3(-15.0f, 0.0f, -3.0f);
                hillScale = 0.5f;
            } else if (i == 2) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 1.0f);
                hillScale = 0.5f;
            } else if (i == 3) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 10.0f);
                hillScale = 0.5f;
            } else if (i == 4) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 14.0f);
                hillScale = 0.5f;
            } else if (i == 5) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 18.0f);
                hillScale = 0.5f;
            } else if (i == 6) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 22.0f);
                hillScale = 0.5f;
            } else if (i == 7) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 26.0f);
                hillScale = 0.5f;
            } else if (i == 8) {
                hillPosition = glm::vec3(15.0f, 0.0f, -7.0f);
                hillScale = 0.5f;
            } else if (i == 9) {
                hillPosition = glm::vec3(15.0f, 0.0f, -3.0f);
                hillScale = 0.5f;
            } else if (i == 10) {
                hillPosition = glm::vec3(15.0f, 0.0f, 1.0f);
                hillScale = 0.5f;
            } else if (i == 11) {
                hillPosition = glm::vec3(15.0f, 0.0f, 10.0f);
                hillScale = 0.5f;
            } else if (i == 12) {
                hillPosition = glm::vec3(15.0f, 0.0f, 14.0f);
                hillScale = 0.5f;
            } else if (i == 13) {
                hillPosition = glm::vec3(15.0f, 0.0f, 18.0f);
                hillScale = 0.5f;
            } else if (i == 14) {
                hillPosition = glm::vec3(15.0f, 0.0f, 22.0f);
                hillScale = 0.5f;
            } else if (i == 15) {
                hillPosition = glm::vec3(15.0f, 0.0f, 26.0f);
                hillScale = 0.5f;
            } else if (i == 16) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 30.0f);
                hillScale = 0.5f;
            } else if (i == 17) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 34.0f);
                hillScale = 0.5f;
            } else if (i == 18) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 38.0f);
                hillScale = 0.5f;
            } else if (i == 19) {
                hillPosition = glm::vec3(-15.0f, 0.0f, 42.0f);
                hillScale = 0.5f;
            } else if (i == 20) {
                hillPosition = glm::vec3(15.0f, 0.0f, 30.0f);
                hillScale = 0.5f;
            } else if (i == 21) {
                hillPosition = glm::vec3(15.0f, 0.0f, 34.0f);
                hillScale = 0.5f;
            } else if (i == 22) {
                hillPosition = glm::vec3(15.0f, 0.0f, 38.0f);
                hillScale = 0.5f;
            } else if (i == 23) {
                hillPosition = glm::vec3(15.0f, 0.0f, 42.0f);
                hillScale = 0.5f;
            }

            // Keep texture density roughly constant w.r.t. mesh scaling
            glUniform1f(uvScaleLocation, 10.0f / hillScale);

            glm::mat4 hillModel = glm::translate(glm::mat4(1.0f), hillPosition) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(hillScale));

            setWorldMatrix(texturedShaderProgram, hillModel);
            setProjectionMatrix(texturedShaderProgram, projection);
            setViewMatrix(texturedShaderProgram, view);
            glBindVertexArray(hillData.VAO); // or whatever your VAO is
            glDrawElements(GL_TRIANGLES, hillData.indexCount, GL_UNSIGNED_INT, 0);
        }

        // Draw the road
        
        glm::mat4 roadModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.001f, -50.0f)) *
                            glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 0.01f, 100.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asphaltTextureID);
        glUniform1i(textureSamplerLocation, 0);
        glUniform1f(uvScaleLocation, 1.0f); // road UV scale

        setWorldMatrix(texturedShaderProgram, roadModel);
        setProjectionMatrix(texturedShaderProgram, projection);
        setViewMatrix(texturedShaderProgram, view);
        glBindVertexArray(roadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw the light poles along the track
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightPoleTextureID);
        glUniform1i(textureSamplerLocation, 0);
        glUniform1f(uvScaleLocation, 1.0f);

        for (int i = 0; i < 16; ++i) {
            glm::vec3 polePosition;
            float poleScale = 0.3f;

            if (i < 8) {
                // left side
                polePosition = glm::vec3(-8.0f, 3.0f, -7.0f + i * 6.0f);
            } else {
                // right side
                polePosition = glm::vec3(8.0f, 3.0f, -7.0f + (i - 8) * 6.0f);
            }

            glm::mat4 poleModel = glm::translate(glm::mat4(1.0f), polePosition) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(poleScale));
            setWorldMatrix(texturedShaderProgram, poleModel);
            setProjectionMatrix(texturedShaderProgram, projection);
            setViewMatrix(texturedShaderProgram, view);
            glBindVertexArray(lightPoleData.VAO);
            glDrawElements(GL_TRIANGLES, lightPoleData.indexCount, GL_UNSIGNED_INT, 0);
        }


        // Draw textured curbs
        setProjectionMatrix(texturedShaderProgram, projection);
        setViewMatrix(texturedShaderProgram, view);
        glBindVertexArray(curbVAO);
        glBindTexture(GL_TEXTURE_2D, curbTextureID);   // red-white texture
        glUniform1i(glGetUniformLocation(texturedShaderProgram,"textureSampler"),0);
        glUniform1f(uvScaleLocation, 1.0f); // curb UV scale

        const float curbW = 0.30f;
        const float halfRoad = 1.5f;
        const float halfCurb = curbW * 0.5f;
        float offset = halfRoad + halfCurb;

        // left side
        glm::mat4 curbL = glm::translate(glm::mat4(1.0f),
                        glm::vec3(-offset, 0.003f, 0.0f)) *
                        glm::scale(glm::mat4(1.0f),
                        glm::vec3(curbW, 0.01f, 100.0f));
        setWorldMatrix(texturedShaderProgram, curbL);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // right side  (same texture, mirrored)
        glm::mat4 curbR = glm::translate(glm::mat4(1.0f),
                        glm::vec3( offset, 0.003f, 0.0f)) *
                        glm::scale(glm::mat4(1.0f),
                        glm::vec3(curbW, 0.01f, 100.0f));
        setWorldMatrix(texturedShaderProgram, curbR);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Draw the Cybertruck (centered and scaled)
        glUseProgram(shaderProgram);
        setProjectionMatrix(shaderProgram, projection);
        setViewMatrix(shaderProgram, view);
        setWorldMatrix(shaderProgram, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 9.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)));
        glBindVertexArray(cybertruckData.VAO);
        glDrawElements(GL_TRIANGLES, cybertruckData.indexCount, GL_UNSIGNED_INT, 0); // Draw the Cybertruck model

        glBindVertexArray(0); // Unbind VAO

        // Draw the Bird model
        float angle = glm::radians(glfwGetTime() * 60.0f); // Rotate the bird model
        glm::mat4 birdModelMatrix = glm::mat4(1.0f);
        birdModelMatrix = glm::translate(birdModelMatrix, glm::vec3(0.0f, 2.0f, 2.0f));
        birdModelMatrix = glm::rotate(birdModelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        birdModelMatrix = glm::translate(birdModelMatrix, glm::vec3(2.0f, 0.0f, 0.0f)); // Position the bird above the ground
        birdModelMatrix = glm::rotate(birdModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        birdModelMatrix = glm::rotate(birdModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate the bird model to face upwards
        birdModelMatrix = glm::scale(birdModelMatrix, glm::vec3(0.001f));

        setWorldMatrix(shaderProgram, birdModelMatrix);
        glBindVertexArray(birdData.VAO);
        glDrawElements(GL_TRIANGLES, birdData.indexCount, GL_UNSIGNED_INT, 0); // Draw the Bird model

        float subAngle = glm::radians(glfwGetTime() * 60.0f); // Rotate the bird model around its own axis
        float radius = 300.0f; // Orbit radius for the second bird
        float yOffset = radius * sin(subAngle); // Calculate the y offset based on the angle
        float zOffset = radius * cos(subAngle); // Calculate the z offset based on the angle
        glm::mat4 bird2Matrix = glm::mat4(1.0f);
        // Translate to position the bird in orbit (around the first bird at origin)
        bird2Matrix = glm::translate(bird2Matrix, glm::vec3(0.0f, yOffset, zOffset));
        bird2Matrix = glm::scale(bird2Matrix, glm::vec3(1.0f)); 

        glm::mat4 secondBird = birdModelMatrix * bird2Matrix; // Combine transformations

        setWorldMatrix(shaderProgram, secondBird);
        glBindVertexArray(birdData.VAO);
        glDrawElements(GL_TRIANGLES, birdData.indexCount, GL_UNSIGNED_INT, 0); // Draw the second Bird model
        

        if(cameraFirstPerson){
            view = lookAt(cameraPos,  // eye
                                 cameraPos + cameraFront,  // center
                                 cameraUp ); // up
        } else{
            float radius = 5.0f;
            glm::vec3 position = cameraPos - radius * cameraFront; // position is on a sphere around the camera position
            view = lookAt(position,  // eye
                                 position + cameraFront,  // center
                                 cameraUp ); // up
        }

        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

        
        
        glfwSwapBuffers(window); // Swap buffers
        glfwPollEvents(); // Poll for events

        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // Camera controls

        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // Convert to spherical coordinates
        const float cameraAngularSpeed = 8.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * deltaTime;
        cameraVerticalAngle   -= dy * cameraAngularSpeed * deltaTime;
        
        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        
        cameraFront = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
        vec3 cameraSide = cross(cameraFront, vec3(0.0f, 1.0f, 0.0f));
        glm::normalize(cameraSide);


        // Speed multiplier for shift
        float speedMultiplier = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 3.0f : 1.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            cameraPos += cameraFront * deltaTime * 1.0f * speedMultiplier; // Move forward
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            cameraPos -= cameraFront * deltaTime * 1.0f * speedMultiplier; // Move backward
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            cameraPos -= cameraSide * deltaTime * 1.0f * speedMultiplier; // Move left
        } 
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            cameraPos += cameraSide * deltaTime * 1.0f * speedMultiplier; // Move right
        }

        // 1st person and 3rd person camera toggle
        
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) 
        {
            cameraFirstPerson = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) 
        {
            cameraFirstPerson = false;
        }


    }
    
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteVertexArrays(1, &roadVAO);
    glDeleteVertexArrays(1, &curbVAO);

    
    glfwTerminate(); // Terminate GLFW
    return 0;
}