#include <iostream>
#include "vertexData.h"


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>


const char* getVertexShaderSource()
{
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;"
        "layout (location = 1) in vec3 aColor;"
        ""
        "out vec3 vertexColor;"
        "uniform mat4 world;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        ""
        "void main()\n"
        "{\n"
        "   vertexColor = aColor;\n"
        "   gl_Position = projection * view * world * vec4(aPos, 1.0);\n"
        "}\n";
}


const char* getFragmentShaderSource()
{
    return
        "#version 330 core\n"
        "in vec3 vertexColor;"
        "out vec4 FragColor;"
        "void main()"
        "{"
        "   FragColor = vec4(vertexColor, 1.0);"
        "}";
}


int compileAndLinkShaders()
{
         // vertex shader
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexShaderSource = getVertexShaderSource();
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
        const char* fragmentShaderSource = getFragmentShaderSource();
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

// Camera variables
glm::vec3 cameraPos   = glm::vec3(0.0f, 1.5f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 cameraSide = glm::cross(cameraFront, cameraUp);
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

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
    

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders();
    
    // Define and upload geometry to the GPU here ...
    GLuint cubeVAO = createVAO(cubeVertices, sizeof(cubeVertices));
    GLuint floorVAO = createVAO(floorVertices, sizeof(floorVertices));

    GLint modelLocation = glGetUniformLocation(shaderProgram, "world");
    GLint viewLocation  = glGetUniformLocation(shaderProgram, "view");
    GLint projLocation  = glGetUniformLocation(shaderProgram, "projection");

    // Set the projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f/600.f, 0.1f, 100.0f);
    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Frame time calculation
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen
        
        glUseProgram(shaderProgram); // Use our shader program

        // Set the view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

        // Set the projection matrix
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

        // Get the location of the color uniform
        int colorLocation = glGetUniformLocation(shaderProgram, "vertexColor");

        // Draw the floor
        glm::mat4 floorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.01f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 0.02f, 1000.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &floorModel[0][0]);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw the cube
        glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f * currentFrame), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &cubeModel[0][0]);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices for a cube
        
       
        
        glfwSwapBuffers(window); // Swap buffers
        glfwPollEvents(); // Poll for events

        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // Camera controls
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            cameraPos += cameraFront * deltaTime * 1.0f; // Move forward
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            cameraPos -= cameraFront * deltaTime * 1.0f; // Move backward
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            cameraPos -= cameraSide * deltaTime * 1.0f; // Move left
        } 
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            cameraPos += cameraSide * deltaTime * 1.0f; // Move right
        }


    }
    
    glDeleteVertexArrays(1, &cubeVAO);
    
    glfwTerminate(); // Terminate GLFW
    return 0;
}