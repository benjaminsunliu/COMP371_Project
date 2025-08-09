#pragma once

const float tilting = 100.0f;

inline float floorVertices[] = {
    // positions         // colors        // UVs            // normals
    -5.0f, 0.0f, -5.0f,   1, 1, 1,        0.0f, 0.0f,           0.0f, 1.0f, 0.0f,
     5.0f, 0.0f,  5.0f,   1, 1, 1,        tilting, tilting,     0.0f, 1.0f, 0.0f,
     5.0f, 0.0f, -5.0f,   1, 1, 1,        tilting, 0.0f,        0.0f, 1.0f, 0.0f,

     5.0f, 0.0f,  5.0f,   1, 1, 1,        tilting, tilting,     0.0f, 1.0f, 0.0f,
    -5.0f, 0.0f, -5.0f,   1, 1, 1,        0.0f, 0.0f,           0.0f, 1.0f, 0.0f,
    -5.0f, 0.0f,  5.0f,   1, 1, 1,        0.0f, tilting,        0.0f, 1.0f, 0.0f
};

const float roadTilting = 100.0f;

inline float roadVertices[] = {
    // Positions          // Colors         // UVs          // Normals
    -0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   0.0f, 0.0f,         0.0f, 1.0f, 0.0f,     // bottom-left    
     0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   1.0f, roadTilting,  0.0f, 1.0f, 0.0f,   // top-right
     0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   1.0f, 0.0f,         0.0f, 1.0f, 0.0f,  // bottom-right

    -0.5f, 0.0f, 0.0f,    0.5f,0.5f,0.5f,   0.0f, 0.0f,         0.0f, 1.0f, 0.0f,   // bottom-left
    -0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   0.0f, roadTilting,  0.0f, 1.0f, 0.0f,   // top-left
     0.5f, 0.0f, 1.0f,    0.5f,0.5f,0.5f,   1.0f, roadTilting,  0.0f, 1.0f, 0.0f   // top-right
};

// ─── Racing-curb quad (2 triangles, POS-COL-UV) ────────────────
 inline float curbVerts[] = {
    //   x     y     z       r  g  b      u     v       // normals
    -0.5f, 0.0f,  0.5f,    1, 1, 1,     0.0f,  0.0f,    0.0f, 1.0f, 0.0f,   // TL
     0.5f, 0.0f, -0.5f,    1, 1, 1,     1.0f, 50.0f,    0.0f, 1.0f, 0.0f,   // BR
    -0.5f, 0.0f, -0.5f,    1, 1, 1,     0.0f, 50.0f,    0.0f, 1.0f, 0.0f,   // BL

    -0.5f, 0.0f,  0.5f,    1, 1, 1,     0.0f,  0.0f,   0.0f, 1.0f, 0.0f,    // TL
     0.5f, 0.0f,  0.5f,    1, 1, 1,     1.0f,  0.0f,   0.0f, 1.0f, 0.0f,    // TR
     0.5f, 0.0f, -0.5f,    1, 1, 1,     1.0f, 50.0f,   0.0f, 1.0f, 0.0f     // BR
};


