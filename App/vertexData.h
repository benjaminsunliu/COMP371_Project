#pragma once

// Cube vertices: positions (x, y, z) + colors (r, g, b)
inline float cubeVertices[] = {
    // positions          // colors
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
};

const float tilting = 5000.0f;

inline float floorVertices[] = {
    // positions        // colors        // UVs (multiplied by tiling factor 10)
    -5.0f, 0.0f, -5.0f, 1, 1, 1,        0.0f, 0.0f,
     5.0f, 0.0f, -5.0f, 1, 1, 1,        tilting, 0.0f,
     5.0f, 0.0f,  5.0f, 1, 1, 1,        tilting, tilting,

     5.0f, 0.0f,  5.0f, 1, 1, 1,        tilting, tilting,
    -5.0f, 0.0f, -5.0f, 1, 1, 1,        0.0f, 0.0f,
    -5.0f, 0.0f,  5.0f, 1, 1, 1,        0.0f, tilting
};

const float roadTilting = 100.0f;

float roadVertices[] = {
    // Positions         // Colors           // UVs
    -0.5f, 0.0f, 0.0f,   0.5f,0.5f,0.5f,    0.0f, 0.0f,    // bottom-left
     0.5f, 0.0f, 0.0f,   0.5f,0.5f,0.5f,    1.0f, 0.0f,    // bottom-right
     0.5f, 0.0f, 1.0f,   0.5f,0.5f,0.5f,    1.0f, roadTilting,   // top-right

    -0.5f, 0.0f, 0.0f,   0.5f,0.5f,0.5f,    0.0f, 0.0f,    // bottom-left
     0.5f, 0.0f, 1.0f,   0.5f,0.5f,0.5f,    1.0f, roadTilting,   // top-right
    -0.5f, 0.0f, 1.0f,   0.5f,0.5f,0.5f,    0.0f, roadTilting    // top-left
};



