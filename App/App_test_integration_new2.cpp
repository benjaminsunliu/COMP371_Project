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

// Cloud quad for sky/clouds
float skyQuad[] = {
    // positions        // colors       // uvs
    -1.0f,  1.0f, 0.0f,  1, 1, 1,  0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  1, 1, 1,  0.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1, 1, 1,  1.0f, 1.0f,
     1.0f, -1.0f, 0.0f,  1, 1, 1,  1.0f, 0.0f
};

// Mouse state
double lastX = 0.0f, lastY = 0.0f;
float yaw = 0.0f;
bool firstMouse = true;

// Car State 
glm::vec3 carPos = glm::vec3(0.0f, 0.0f, 10.0f);
float carYaw = 0.0f;
float wheelAngle = 0.0f;
float steerAngle = 0.0f;
GLsizei wheelIndexCount;



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
        "uniform bool useBlackKey;"
        "out vec4 FragColor;"
        "void main()"
        "{"
        "    vec4 tex = texture(textureSampler, vertexUV);"
        "    if (useBlackKey) {"
        "        if (tex.r < 0.05 && tex.g < 0.05 && tex.b < 0.05) discard;"
        "        FragColor = vec4(tex.rgb, 1.0);"
        "    } else {"
        "        FragColor = tex;"
        "    }"
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

void createCubeVAO(GLuint &VAO, GLuint &VBO, GLuint &EBO) {
    float vertices[] = {
        // positions       normals     texcoords
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,
         0.5f,-0.5f, 0.5f, 0,0,1, 1,0,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
        -0.5f, 0.5f, 0.5f, 0,0,1, 0,1,

        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
         0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
        -0.5f, 0.5f,-0.5f, 0,0,-1, 0,1
    };
    GLuint indices[] = {
        0,1,2, 2,3,0,
        1,5,6, 6,2,1,
        5,4,7, 7,6,5,
        4,0,3, 3,7,4,
        3,2,6, 6,7,3,
        4,5,1, 1,0,4
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// -------------------- Trapezoid VAO (Cabin) --------------------
void createCabinVAO(GLuint &VAO, GLuint &VBO, GLuint &EBO) {
    float vertices[] = {
        // positions         normals  tex
        -0.5f,-0.5f, 0.5f,   0,0,1,   0,0,
         0.5f,-0.5f, 0.5f,   0,0,1,   1,0,
         0.3f, 0.5f, 0.3f,   0,0,1,   0.8,1,
        -0.3f, 0.5f, 0.3f,   0,0,1,   0.2,1,

        -0.5f,-0.5f,-0.5f,   0,0,-1,  0,0,
         0.5f,-0.5f,-0.5f,   0,0,-1,  1,0,
         0.3f, 0.5f,-0.3f,   0,0,-1,  0.8,1,
        -0.3f, 0.5f,-0.3f,   0,0,-1,  0.2,1
    };
    GLuint indices[] = {
        0,1,2, 2,3,0,
        1,5,6, 6,2,1,
        5,4,7, 7,6,5,
        4,0,3, 3,7,4,
        3,2,6, 6,7,3
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void createWheelVAO(GLuint &VAO, GLuint &VBO, GLuint &EBO, int segments = 32) {
    std::vector<float> verts;
    std::vector<unsigned int> inds;

    float radius = 0.5f;
    float width  = 0.3f;
    float halfW  = width / 2.0f;

    // ===== 1. Cylinder side vertices (outer tire wall) =====
    for (int i = 0; i <= segments; ++i) {
        float theta = (2.0f * M_PI * i) / segments;
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        float u = (float)i / segments; // horizontal wrap 0..1

        // Front side (z = halfW)
        verts.insert(verts.end(), { x, y,  halfW,  1,1,1,  u, 1 });
        // Back side (z = -halfW)
        verts.insert(verts.end(), { x, y, -halfW,  1,1,1,  u, 0 });
    }

    // Create indices for cylinder wall (two triangles per quad)
    for (int i = 0; i < segments; ++i) {
        int start = i * 2;
        inds.push_back(start);
        inds.push_back(start + 1);
        inds.push_back(start + 2);

        inds.push_back(start + 1);
        inds.push_back(start + 3);
        inds.push_back(start + 2);
    }

    // ===== 2. Caps (front and back discs) =====
    int centerFrontIndex = verts.size() / 8;
    // Center vertex front
    verts.insert(verts.end(), { 0, 0,  halfW,  1,1,1,  0.5f, 0.5f });

    int centerBackIndex = centerFrontIndex + 1;
    // Center vertex back
    verts.insert(verts.end(), { 0, 0, -halfW,  1,1,1,  0.5f, 0.5f });

    // Front face fan
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        inds.push_back(centerFrontIndex);
        inds.push_back(i * 2);
        inds.push_back(next * 2);
    }

    // Back face fan
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        inds.push_back(centerBackIndex);
        inds.push_back(next * 2 + 1);
        inds.push_back(i * 2 + 1);
    }
    wheelIndexCount = inds.size();

    // Upload to GPU
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
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
    // Load grandstand texture before main loop
    GLuint grandstandTextureID = loadTexture("Textures/generic medium_01_a.png");
    GLuint grandstandTextureB = loadTexture("Textures/generic medium_01_b.png");
    GLuint grandstandTextureC = loadTexture("Textures/generic medium_01_c.png");
    GLuint carTexture = loadTexture("Textures/car_wrap.jpg");
    GLuint tireTexture = loadTexture("Textures/tires.jpg");
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Cloud setup (must be after GLEW init)
    GLuint cloudTexture1 = loadTexture("Textures/01.png");
    GLuint cloudTexture2 = loadTexture("Textures/02.png");
    GLuint cloudTexture3 = loadTexture("Textures/03.png");
    GLuint cloudVAO = createTexturedVAO(skyQuad, sizeof(skyQuad));

    struct Cloud {
        GLuint textureID;
        glm::vec3 position;
        float speed;
        float scale;
    };
    std::vector<Cloud> clouds = {
        {cloudTexture1, glm::vec3(-30.0f, 12.0f, 40.0f), 0.5f, 4.0f},
        {cloudTexture2, glm::vec3(25.0f, 14.0f, 30.0f), 0.3f, 5.0f},
        {cloudTexture3, glm::vec3(0.0f, 11.0f, 60.0f), 0.4f, 3.5f},
        {cloudTexture1, glm::vec3(10.0f, 13.0f, -10.0f), 0.4f, 4.2f},
        {cloudTexture2, glm::vec3(-15.0f, 15.0f, -20.0f), 0.3f, 3.8f},
        {cloudTexture3, glm::vec3(-50.0f, 13.0f, 10.0f), 0.2f, 4.5f},
        {cloudTexture1, glm::vec3(40.0f, 16.0f, -35.0f), 0.3f, 3.9f},
        {cloudTexture2, glm::vec3(-20.0f, 14.5f, -50.0f), 0.4f, 5.2f},
        {cloudTexture3, glm::vec3(30.0f, 12.5f, 20.0f), 0.3f, 4.1f},
        {cloudTexture1, glm::vec3(-10.0f, 15.0f, 0.0f), 0.5f, 4.8f},

        {cloudTexture2, glm::vec3(5.0f, 13.5f, 15.0f), 0.3f, 3.7f},
        {cloudTexture3, glm::vec3(-25.0f, 14.0f, -15.0f), 0.4f, 4.5f},
        {cloudTexture1, glm::vec3(20.0f, 13.0f, 5.0f), 0.5f, 4.3f},
        {cloudTexture3, glm::vec3(0.0f, 16.0f, -30.0f), 0.3f, 4.8f},
        {cloudTexture2, glm::vec3(15.0f, 15.0f, 45.0f), 0.2f, 3.6f},

        {cloudTexture2, glm::vec3(7.5f, 9.2f, 53.5f), 0.4f, 4.2f},
        {cloudTexture3, glm::vec3(-12.0f, 6.0f, 57.0f), 0.3f, 3.9f},
    };

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

    GLuint carBodyVAO, carBodyVBO, carBodyEBO;
    createCubeVAO(carBodyVAO, carBodyVBO, carBodyEBO);

    GLuint cabinVAO, cabinVBO, cabinEBO;
    createCabinVAO(cabinVAO, cabinVBO, cabinEBO);

    GLuint wheelVAO, wheelVBO, wheelEBO;
    createWheelVAO(wheelVAO, wheelVBO, wheelEBO);
   

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
    // Load the grandstand model using the Assimp loader
    ModelData grandstandData = loadModelWithAssimp("Models/generic medium.obj");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Frame time calculation
        float deltaTime = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

        // Get the location of the color uniform
        int colorLocation = glGetUniformLocation(shaderProgram, "vertexColor");

        // --- CLOUDS DRAWING (before other objects) ---
        glUseProgram(texturedShaderProgram);
        GLuint textureSamplerLocation = glGetUniformLocation(texturedShaderProgram, "textureSampler");
        GLint uvScaleLocation = glGetUniformLocation(texturedShaderProgram, "uvScale");
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLint useBlackKeyLoc = glGetUniformLocation(texturedShaderProgram, "useBlackKey");
        glUniform1i(useBlackKeyLoc, GL_FALSE);

        for (auto& cloud : clouds) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cloud.textureID);
            glUniform1i(textureSamplerLocation, 0);
            glUniform1f(uvScaleLocation, 1.0f);
            // Y-axis-constrained billboarding: make the cloud face the camera
            glm::vec3 cloudToCamera = glm::normalize(cameraPos - cloud.position);
            glm::mat4 billboardRotation = glm::inverse(glm::lookAt(glm::vec3(0), cloudToCamera, glm::vec3(0, 1, 0)));
            billboardRotation[3] = glm::vec4(0, 0, 0, 1); // clear translation
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cloud.position) *
                              billboardRotation *
                              glm::scale(glm::mat4(1.0f), glm::vec3(cloud.scale));
            setWorldMatrix(texturedShaderProgram, model);
            setProjectionMatrix(texturedShaderProgram, projection);
            setViewMatrix(texturedShaderProgram, view);
            glBindVertexArray(cloudVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glUniform1i(useBlackKeyLoc, GL_FALSE);
        glDisable(GL_BLEND);

        // --- END CLOUDS ---

        glUseProgram(texturedShaderProgram);
        // textureSamplerLocation and uvScaleLocation already obtained above
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


        // Draw the grandstands after rendering the light poles, rotating textures for variety
        std::vector<glm::vec3> grandstandPositions;
        for (float z = -45.0f; z <= 45.0f; z += 10.0f) {
            grandstandPositions.push_back(glm::vec3(-6.0f, 0.0f, z)); // Left side
            grandstandPositions.push_back(glm::vec3( 6.0f, 0.0f, z)); // Right side
        }

        std::vector<GLuint> grandstandTextures = {
            grandstandTextureID,
            grandstandTextureB,
            grandstandTextureC,
            grandstandTextureID
        };

        for (size_t i = 0; i < grandstandPositions.size(); ++i) {
            glBindTexture(GL_TEXTURE_2D, grandstandTextures[i % grandstandTextures.size()]);
            glUniform1i(textureSamplerLocation, 0);
            glUniform1f(uvScaleLocation, 1.0f);

            // Corrected rotation: x > 0.0f gets 270, else 90
            float angle = (grandstandPositions[i].x > 0.0f) ? 270.0f : 90.0f;
            glm::mat4 grandstandModel = glm::translate(glm::mat4(1.0f), grandstandPositions[i]) *
                                        glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                        glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));  // Increased scale for visibility
            setWorldMatrix(texturedShaderProgram, grandstandModel);
            setProjectionMatrix(texturedShaderProgram, projection);
            setViewMatrix(texturedShaderProgram, view);
            glBindVertexArray(grandstandData.VAO);
            glDrawElements(GL_TRIANGLES, grandstandData.indexCount, GL_UNSIGNED_INT, 0);
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

        glUseProgram(texturedShaderProgram);

        // Car Body
        glm::mat4 bodyModel = glm::translate(glm::mat4(1.0f), carPos + glm::vec3(0, 0.5f, 0));
        bodyModel = glm::scale(bodyModel, glm::vec3(2.5f, 0.7f, 5.0f));
        setWorldMatrix(texturedShaderProgram, bodyModel);
        glBindTexture(GL_TEXTURE_2D, carTexture);
        glBindVertexArray(carBodyVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Cabin
        glm::mat4 cabinModel = glm::translate(glm::mat4(1.0f), carPos + glm::vec3(0, 1.0f, 0));
        cabinModel = glm::scale(cabinModel, glm::vec3(1.5f, 0.8f, 4.0f));
        setWorldMatrix(texturedShaderProgram, cabinModel);
        glBindVertexArray(cabinVAO);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

        // Wheels
        float wheelX = 1.3f, wheelZ = 2.0f;
        for (int i = -1; i <= 1; i += 2) {
            for (int j = -1; j <= 1; j += 2) {
                glm::vec3 offset(i * wheelX, 0.5f, j * wheelZ);
                glm::mat4 wheelModel = glm::translate(glm::mat4(1.0f), carPos + offset);
                wheelModel = glm::rotate(wheelModel, glm::radians(90.0f), glm::vec3(0, 1, 0));
                if (j == 1) wheelModel = glm::rotate(wheelModel, glm::radians(steerAngle), glm::vec3(0, 1, 0));
                wheelModel = glm::rotate(wheelModel, glm::radians(wheelAngle), glm::vec3(0, 0, 1));
                wheelModel = glm::scale(wheelModel, glm::vec3(0.7f));
                setWorldMatrix(texturedShaderProgram, wheelModel);
                glBindTexture(GL_TEXTURE_2D, tireTexture);
                glBindVertexArray(wheelVAO);
                glDrawElements(GL_TRIANGLES, wheelIndexCount, GL_UNSIGNED_INT, 0);
            }
        }
        
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

        // Car commands
        float carSpeed = 5.0f * deltaTime;
        float wheelSpinSpeed = 120.0f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            carPos += carSpeed * glm::vec3(sin(glm::radians(carYaw)), 0.0f, -cos(glm::radians(carYaw)));
            wheelAngle -= wheelSpinSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            carPos -= carSpeed * glm::vec3(sin(glm::radians(carYaw)), 0.0f, -cos(glm::radians(carYaw)));
            wheelAngle += wheelSpinSpeed;
        }

        // Steering (J = left, L = right)
        steerAngle = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) steerAngle = 25.0f;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) steerAngle = -25.0f;

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
    glDeleteVertexArrays(1, &carBodyVAO);
    glDeleteVertexArrays(1, &cabinVAO);
    glDeleteVertexArrays(1, &wheelVAO);

    
    glfwTerminate(); // Terminate GLFW
    return 0;
}