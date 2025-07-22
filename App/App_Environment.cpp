//  App_Environment.cpp  – Textured environment with free‑roam camera

#include <iostream>
#include <cmath>
#include "vertexData.h"              // cube & floor, unchanged
#include "CarVertex/CyberTruck.h"    // the truck mesh (pos+colour)

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

/*──────────────────────────── texture loader (stb) ───────────────────────*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
static GLuint loadTexture(const char* path,bool repeat=true)
{
    int w,h,n; stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path,&w,&h,&n,0);
    if(!data){ std::cerr<<"Texture load failed: "<<path<<"\n"; return 0; }
    GLuint tex; glGenTextures(1,&tex); glBindTexture(GL_TEXTURE_2D,tex);
    GLenum fmt = (n==3?GL_RGB:GL_RGBA);
    glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    return tex;
}

/*──────────────────────────── mouse & camera ─────────────────────────────*/
double gLastX=0, gLastY=0; bool gFirstMouse=true; float gYaw=0.0f, gPitch=0.0f;
vec3 gCamPos(0.f,1.5f,5.f), gCamFront(0.f,0.f,-1.f), gCamUp(0.f,1.f,0.f);
vec3 gCamSide;
static void updateCameraVectors(){
    vec3 dir;
    dir.x = cos(radians(gYaw))*cos(radians(gPitch));
    dir.y = sin(radians(gPitch));
    dir.z = -sin(radians(gYaw))*cos(radians(gPitch));
    gCamFront = normalize(dir);
    gCamSide  = normalize(cross(gCamFront,gCamUp));
}
static void mouse_callback(GLFWwindow* win,double xpos,double ypos){
    const float sens=0.1f;
    if(glfwGetMouseButton(win,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){
        if(gFirstMouse){ gLastX=xpos; gLastY=ypos; gFirstMouse=false; }
        float dx = (float)(xpos-gLastX), dy = (float)(ypos-gLastY);
        gLastX=xpos; gLastY=ypos;
        gYaw   += dx*sens;
        gPitch -= dy*sens;  gPitch = clamp(gPitch,-89.f,89.f);
        updateCameraVectors();
    }else gFirstMouse=true;
}

/*──────────────────────── ground & track (pos + uv) ──────────────────────*/
static const float gGround[] = {  // two triangles, tiled 100×
    -50,0,-50,   0,0,   50,0,-50, 100,0,   50,0,50,  100,100,
     50,0, 50, 100,100, -50,0,50,   0,100, -50,0,-50,  0,0 };
static const float gTrack[] = {   // 24 tri (12 quads) around centre, uv 0‑1 per quad
    // quad 1
    -20,0.01,-10,0,0,  20,0.01,-10,1,0,  20,0.01,-6,1,1,
     20,0.01,-6,1,1, -20,0.01,-6,0,1, -20,0.01,-10,0,0,
    // quad 2
    -20,0.01,6,0,0,  20,0.01,6,1,0,  20,0.01,10,1,1,
     20,0.01,10,1,1, -20,0.01,10,0,1, -20,0.01,6,0,0,
    // quad 3
    -20,0.01,-6,0,0, -6,0.01,-6,1,0, -6,0.01,6,1,1,
     -6,0.01,6,1,1, -20,0.01,6,0,1, -20,0.01,-6,0,0,
    // quad 4
      6,0.01,-6,0,0, 20,0.01,-6,1,0, 20,0.01,6,1,1,
     20,0.01,6,1,1,  6,0.01,6,0,1,   6,0.01,-6,0,0 };

/*──────────────────────────── shaders ─────────────────────────────────────*/
static const char* texVtx = R"(#version 330 core
layout(location=0)in vec3 aPos; layout(location=1)in vec2 aUV;
uniform mat4 world,view,projection; out vec2 vUV;
void main(){ vUV=aUV; gl_Position=projection*view*world*vec4(aPos,1);} )";
static const char* texFrag = R"(#version 330 core
in vec2 vUV; uniform sampler2D tex; out vec4 FragColor;
void main(){ FragColor = texture(tex,vUV); })";
static const char* colVtx = R"(#version 330 core
layout(location=0)in vec3 aPos; layout(location=1)in vec3 aCol;
uniform mat4 world,view,projection; out vec3 vCol;
void main(){ vCol=aCol; gl_Position=projection*view*world*vec4(aPos,1);} )";
static const char* colFrag = R"(#version 330 core
in vec3 vCol; out vec4 FragColor; void main(){ FragColor=vec4(vCol,1);} )";
static GLuint compile(GLenum t,const char* s){ GLuint id=glCreateShader(t); glShaderSource(id,1,&s,nullptr); glCompileShader(id); int ok; glGetShaderiv(id,GL_COMPILE_STATUS,&ok); if(!ok){ char log[512]; glGetShaderInfoLog(id,512,nullptr,log); std::cerr<<log<<"\n";} return id; }
static GLuint link(GLuint vs,GLuint fs){ GLuint p=glCreateProgram(); glAttachShader(p,vs); glAttachShader(p,fs); glLinkProgram(p); int ok; glGetProgramiv(p,GL_LINK_STATUS,&ok); if(!ok){ char log[512]; glGetProgramInfoLog(p,512,nullptr,log); std::cerr<<log<<"\n";} glDeleteShader(vs); glDeleteShader(fs); return p; }

/*──────────────── VAO helpers (pos+uv OR pos+col) ────────────────────────*/
static GLuint makeVAO(const float* data,size_t bytes,int stride){
    GLuint vao,vbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,bytes,data,GL_STATIC_DRAW);
    if(stride==5){ // pos+uv
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }else if(stride==6){ // pos+col
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(0);
    return vao;
}

/*────────────────────────────── main ─────────────────────────────────────*/
int main(){
    // GLFW
    if(!glfwInit()){ std::cerr<<"GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(1280,720,"COMP371 – Textured Env",nullptr,nullptr);
    if(!win){ glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetCursorPosCallback(win,mouse_callback);
    glfwSetFramebufferSizeCallback(win,[](GLFWwindow*,int w,int h){ glViewport(0,0,w,h); });

    glewExperimental = GL_TRUE; if(glewInit()!=GLEW_OK){ std::cerr<<"GLEW init failed\n"; return -1; }
    glEnable(GL_DEPTH_TEST); glClearColor(0.52f,0.75f,0.95f,1); // sky blue

    // programs
    GLuint progTex = link(compile(GL_VERTEX_SHADER,texVtx), compile(GL_FRAGMENT_SHADER,texFrag));
    GLuint progCol = link(compile(GL_VERTEX_SHADER,colVtx), compile(GL_FRAGMENT_SHADER,colFrag));

    // VAOs
    GLuint vaoGround = makeVAO(gGround,sizeof gGround,5);
    GLuint vaoTrack  = makeVAO(gTrack ,sizeof gTrack ,5);
    GLuint vaoTruck  = makeVAO(cybertruckVertices,sizeof cybertruckVertices,6);

    // textures
    GLuint texGrass  = loadTexture("Textures/grass.jpeg");
    GLuint texAsphalt= loadTexture("Textures/asphalt.jpg");

    // uniform indices
    GLint wT = glGetUniformLocation(progTex,"world");
    GLint vT = glGetUniformLocation(progTex,"view");
    GLint pT = glGetUniformLocation(progTex,"projection");
    GLint sT = glGetUniformLocation(progTex,"tex");

    GLint wC = glGetUniformLocation(progCol,"world");
    GLint vC = glGetUniformLocation(progCol,"view");
    GLint pC = glGetUniformLocation(progCol,"projection");

    // projection (once)
    int fbw,fbh; glfwGetFramebufferSize(win,&fbw,&fbh);
    mat4 projection = perspective(radians(45.f),(float)fbw/fbh,0.1f,200.f);

    glUseProgram(progTex); glUniformMatrix4fv(pT,1,GL_FALSE,&projection[0][0]); glUniform1i(sT,0);
    glUseProgram(progCol); glUniformMatrix4fv(pC,1,GL_FALSE,&projection[0][0]);

    /* main loop */
    float last = (float)glfwGetTime(); updateCameraVectors();
    while(!glfwWindowShouldClose(win)){
        float now=(float)glfwGetTime(), dt=now-last; last=now;
        const float spd=5.f;
        if(glfwGetKey(win,GLFW_KEY_W)==GLFW_PRESS) gCamPos+=gCamFront*spd*dt;
        if(glfwGetKey(win,GLFW_KEY_S)==GLFW_PRESS) gCamPos-=gCamFront*spd*dt;
        if(glfwGetKey(win,GLFW_KEY_A)==GLFW_PRESS) gCamPos-=gCamSide *spd*dt;
        if(glfwGetKey(win,GLFW_KEY_D)==GLFW_PRESS) gCamPos+=gCamSide *spd*dt;
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(win,1);

        mat4 view = lookAt(gCamPos, gCamPos+gCamFront, gCamUp);

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        /* ground */
        glUseProgram(progTex);
        glUniformMatrix4fv(vT,1,GL_FALSE,&view[0][0]);
        mat4 M=mat4(1); glUniformMatrix4fv(wT,1,GL_FALSE,&M[0][0]);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,texGrass);
        glBindVertexArray(vaoGround); glDrawArrays(GL_TRIANGLES,0,6);

        /* track */
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,texAsphalt);
        glBindVertexArray(vaoTrack); glDrawArrays(GL_TRIANGLES,0,24);

        /* truck */
        glUseProgram(progCol);
        glUniformMatrix4fv(vC,1,GL_FALSE,&view[0][0]);
        M = translate(mat4(1),vec3(0,0.5f,0)) * scale(mat4(1),vec3(5)); // stationary
        glUniformMatrix4fv(wC,1,GL_FALSE,&M[0][0]);
        int vTruck = sizeof(cybertruckVertices)/(6*sizeof(float));
        glBindVertexArray(vaoTruck); glDrawArrays(GL_TRIANGLES,0,vTruck);

        glfwSwapBuffers(win); glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
