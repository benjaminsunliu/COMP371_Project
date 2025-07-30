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

const float tilting = 100.0f;

inline float floorVertices[] = {
    // positions         // colors        // UVs
    -5.0f, 0.0f, -5.0f,   1, 1, 1,        0.0f, 0.0f,
     5.0f, 0.0f,  5.0f,   1, 1, 1,        tilting, tilting,
     5.0f, 0.0f, -5.0f,   1, 1, 1,        tilting, 0.0f,

     5.0f, 0.0f,  5.0f,   1, 1, 1,        tilting, tilting,
    -5.0f, 0.0f, -5.0f,   1, 1, 1,        0.0f, 0.0f,
    -5.0f, 0.0f,  5.0f,   1, 1, 1,        0.0f, tilting
};

const float roadTilting = 10.0f;

inline float roadVertices[] = {
    // Positions          // Colors         // UVs
    -0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   0.0f, 0.0f,       // bottom-left
     0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   1.0f, roadTilting, // top-right
     0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   1.0f, 0.0f,       // bottom-right

    -0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   0.0f, 0.0f,       // bottom-left
    -0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   0.0f, roadTilting, // top-left
     0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   1.0f, roadTilting  // top-right
};

// ─── Racing-curb quad (2 triangles, POS-COL-UV) ────────────────
 inline float curbVerts[] = {
    //   x     y     z       r  g  b      u     v
    -0.5f, 0.0f,  0.5f,    1, 1, 1,     0.0f,  0.0f,   // TL
     0.5f, 0.0f, -0.5f,    1, 1, 1,     1.0f, 50.0f,   // BR
    -0.5f, 0.0f, -0.5f,    1, 1, 1,     0.0f, 50.0f,   // BL

    -0.5f, 0.0f,  0.5f,    1, 1, 1,     0.0f,  0.0f,   // TL
     0.5f, 0.0f,  0.5f,    1, 1, 1,     1.0f,  0.0f,   // TR
     0.5f, 0.0f, -0.5f,    1, 1, 1,     1.0f, 50.0f    // BR
};


