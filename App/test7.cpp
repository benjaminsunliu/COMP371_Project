// Required Libraries
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// -------------------- Camera State --------------------
glm::vec3 initialCameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
float initialHorizontalAngle = 90.0f;  // Facing -Z
float initialVerticalAngle = 0.0f;

// Current camera state
glm::vec3 cameraPosition = initialCameraPos;
float cameraHorizontalAngle = initialHorizontalAngle;
float cameraVerticalAngle = initialVerticalAngle;
glm::vec3 cameraLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Mouse tracking
double lastMouseX = WIDTH / 2.0, lastMouseY = HEIGHT / 2.0;
bool leftMouseHeld = false;

// Camera movement speed
float cameraSpeed = 5.0f;


// Car State 
glm::vec3 carPos = glm::vec3(0.0f, 0.0f, 0.0f);
float carYaw = 0.0f;
float wheelAngle = 0.0f;

// Texture IDs 
GLuint carTexture;
GLuint tireTexture;


// -------------------- Projection/View --------------------
glm::mat4 projection;
glm::mat4 view;


// -------------------- Texture Loader --------------------
GLuint loadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return textureID;
}

// -------------------- Functions --------------------

// Draw textured cube (body + cabin)
void drawCubeTextured() {
    glBegin(GL_QUADS);

    // Front
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // Left
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // Right
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

    // Top
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // Bottom
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

    glEnd();
}

void drawCabinTextured() {
    glBegin(GL_QUADS);

    // FRONT (Trapezoid inward) 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);  // bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);  // bottom-right
    glTexCoord2f(0.8f, 1.0f); glVertex3f( 0.3f,  0.5f,  0.3f);  // top-right (inward)
    glTexCoord2f(0.2f, 1.0f); glVertex3f(-0.3f,  0.5f,  0.3f);  // top-left (inward)

    // BACK (Reverse Trapezoid inward) 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);  // bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);  // bottom-right
    glTexCoord2f(0.8f, 1.0f); glVertex3f( 0.3f,  0.5f, -0.3f);  // top-right (inward)
    glTexCoord2f(0.2f, 1.0f); glVertex3f(-0.3f,  0.5f, -0.3f);  // top-left (inward)

    // LEFT (connects front/back trapezoids) 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);  // bottom-back
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);  // bottom-front
    glTexCoord2f(0.8f, 1.0f); glVertex3f(-0.3f,  0.5f,  0.3f);  // top-front (inward)
    glTexCoord2f(0.2f, 1.0f); glVertex3f(-0.3f,  0.5f, -0.3f);  // top-back (inward)

    // --- RIGHT (connects front/back trapezoids) 
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);  // bottom-back
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);  // bottom-front
    glTexCoord2f(0.8f, 1.0f); glVertex3f( 0.3f,  0.5f,  0.3f);  // top-front (inward)
    glTexCoord2f(0.2f, 1.0f); glVertex3f( 0.3f,  0.5f, -0.3f);  // top-back (inward)

    // TOP (connect sloped edges) 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.3f,  0.5f,  0.3f);  // front-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.3f,  0.5f,  0.3f);  // front-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.3f,  0.5f, -0.3f);  // back-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.3f,  0.5f, -0.3f);  // back-left

    glEnd();
}

void drawWheelTextured(float outerRadius = 0.6f, float width = 0.3f, int segments = 32, int spokeCount = 8) {
    float innerRadius = outerRadius * 0.75f;
    float halfWidth   = width / 2.0f;

    // 1. Tread Surface (Outer cylinder)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tireTexture);
    glColor3f(1.0f, 1.0f, 1.0f); // Ensure texture color not tinted

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = outerRadius * cos(angle);
        float y = outerRadius * sin(angle);
        float u = (float)i / segments;

        glTexCoord2f(u, 1.0f); glVertex3f(x, y,  halfWidth);
        glTexCoord2f(u, 0.0f); glVertex3f(x, y, -halfWidth);
    }
    glEnd();


    // 2. RING FACES (Front/Back) 
    for (int side = -1; side <= 1; side += 2) { // -1 = back, 1 = front
        float z = halfWidth * side;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            float xOuter = outerRadius * cos(angle);
            float yOuter = outerRadius * sin(angle);
            float xInner = innerRadius * cos(angle);
            float yInner = innerRadius * sin(angle);
            float u = (float)i / segments;

            glTexCoord2f(u, 1.0f); glVertex3f(xOuter, yOuter, z);
            glTexCoord2f(u, 0.0f); glVertex3f(xInner, yInner, z);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);


// 3. Spokes (Touching Center)

glColor3f(1.0f, 1.0f, 1.0f); // White spokes

float spokeOuterRadius = innerRadius;   // Wide at rim
float spokeAngleWidth  = (2.0f * M_PI / spokeCount) * 0.3f; // width factor

for (int s = 0; s < spokeCount; s++) {
    float baseAngle = 2.0f * M_PI * s / spokeCount;

    // Two edges at outer rim
    float angle1 = baseAngle - spokeAngleWidth;
    float angle2 = baseAngle + spokeAngleWidth;

    // Wide part (outer rim)
    float xOuter1 = spokeOuterRadius * cos(angle1);
    float yOuter1 = spokeOuterRadius * sin(angle1);
    float xOuter2 = spokeOuterRadius * cos(angle2);
    float yOuter2 = spokeOuterRadius * sin(angle2);

    // Center tip (0,0)
    float xCenter = 0.0f;
    float yCenter = 0.0f;

    // FRONT face
    glBegin(GL_TRIANGLES);
        glVertex3f(xOuter1, yOuter1,  halfWidth);
        glVertex3f(xOuter2, yOuter2,  halfWidth);
        glVertex3f(xCenter, yCenter,   halfWidth);
    glEnd();

    // BACK face
    glBegin(GL_TRIANGLES);
        glVertex3f(xOuter2, yOuter2, -halfWidth);
        glVertex3f(xOuter1, yOuter1, -halfWidth);
        glVertex3f(xCenter, yCenter,  -halfWidth);
    glEnd();

    // sides
    glBegin(GL_QUAD_STRIP);
        glVertex3f(xOuter1, yOuter1,  halfWidth);
        glVertex3f(xOuter1, yOuter1, -halfWidth);
        glVertex3f(xOuter2, yOuter2,  halfWidth);
        glVertex3f(xOuter2, yOuter2, -halfWidth);
        glVertex3f(xCenter, yCenter,   halfWidth);
        glVertex3f(xCenter, yCenter,  -halfWidth);
    glEnd();
}


}


// Draw the car (body + cabin textured, wheels partially textured)
void drawCar() {
    float wheelRadius = 0.5f;
    float bodyScaleX = 2.5f, bodyScaleY = 0.7f, bodyScaleZ = 5.0f;
    float cabinScaleX = 1.5f, cabinScaleY = 0.8f, cabinScaleZ = 4.0f;
    float wheelOffsetZ = 2.0f, wheelOffsetX = 1.3f;

    float bodyCenterY = wheelRadius + (bodyScaleY / 2.0f);
    float cabinCenterY = wheelRadius + bodyScaleY + (cabinScaleY / 2.0f);

    // Body + Cabin
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, carTexture);

    // Body
    glPushMatrix();
    glTranslatef(carPos.x, carPos.y + bodyCenterY, carPos.z);
    glScalef(bodyScaleX, bodyScaleY, bodyScaleZ);
    drawCubeTextured();
    glPopMatrix();

    // Cabin
    glPushMatrix();
    glTranslatef(carPos.x, carPos.y + cabinCenterY, carPos.z);
    glScalef(cabinScaleX, cabinScaleY, cabinScaleZ);
    drawCabinTextured();  // cabin
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // Wheels (TIRE, SPOKES) -----
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            glPushMatrix();
            glTranslatef(carPos.x + i * wheelOffsetX,
                         carPos.y + wheelRadius,
                         carPos.z + j * wheelOffsetZ);

            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);

            float wheelScale = wheelRadius * 1.2f;
            glScalef(wheelScale, wheelScale, wheelScale);
            drawWheelTextured();
            glPopMatrix();
        }
    }
}

void drawTrack() {
    srand(42);
    for (int i = 0; i < 50; ++i) {
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -i * 4.0f);
        glScalef(5.0f, 0.1f, 4.0f);
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glEnd();
        glPopMatrix();
    }
}

void processInput(GLFWwindow* window, float deltaTime) {
    float velocity = cameraSpeed * deltaTime;

    glm::vec3 right = glm::normalize(glm::cross(cameraLookAt, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraLookAt * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraLookAt * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += right * velocity;

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

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cameraPosition = initialCameraPos;
        cameraHorizontalAngle = initialHorizontalAngle;
        cameraVerticalAngle = initialVerticalAngle;

        float theta = glm::radians(cameraHorizontalAngle);
        float phi = glm::radians(cameraVerticalAngle);
        cameraLookAt = glm::vec3(cos(phi) * cos(theta),
                                 sin(phi),
                                 -cos(phi) * sin(theta));
    }
}

void handleMouseInput(GLFWwindow* window) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!leftMouseHeld) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            leftMouseHeld = true;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        double dx = mouseX - lastMouseX;
        double dy = lastMouseY - mouseY;

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        const float sensitivity = 0.008f;
        cameraHorizontalAngle += dx * sensitivity;
        cameraVerticalAngle += dy * sensitivity;

        if (cameraHorizontalAngle > 360.0f) cameraHorizontalAngle -= 360.0f;
        if (cameraHorizontalAngle < 0.0f)   cameraHorizontalAngle += 360.0f;

        if (cameraVerticalAngle > 85.0f)  cameraVerticalAngle = 85.0f;
        if (cameraVerticalAngle < -85.0f) cameraVerticalAngle = -85.0f;

        float theta = glm::radians(cameraHorizontalAngle);
        float phi = glm::radians(cameraVerticalAngle);
        cameraLookAt = glm::normalize(glm::vec3(
            cos(phi) * cos(theta),
            sin(phi),
            -cos(phi) * sin(theta)
        ));
    } else {
        if (leftMouseHeld) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            leftMouseHeld = false;
        }
    }
}

// Setup View 
void setupView() {
    projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    view = glm::lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(view));
}

// Main 
int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Car and Free Camera", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Load textures
    carTexture = loadTexture("Textures/car_wrap.jpg");
    tireTexture = loadTexture("Textures/tires.jpg");

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handleMouseInput(window);
        processInput(window, deltaTime);
        setupView();

        drawTrack();
        drawCar();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
