# OpenGL Race Game

A 3D race environment built using modern OpenGL (3.3 core profile) featuring:
- Driveable car with animated wheels and steering
- Dynamic camera system (first- and third-person toggle)
- Textured terrain with road, curbs, and environment elements
- Instanced models: mountains, grandstands, light poles
- Sky system with moving clouds
- Animated birds with hierarchical rotation

## Features

- **Graphics APIs**: OpenGL 3.3 Core Profile with GLEW and GLFW
- **Math**: GLM (camera, transformation matrices)
- **Models**: Assimp to load `.obj` files
- **Textures**: STB Image to load textures
- **Input**: Keyboard (WASD + Shift for camera movement, IJKL for car control, 1/2 for view modes)

## Controls

| Key         | Action                          |
|-------------|----------------------------------|
| `W/A/S/D`   | Move camera (1st/3rd person)     |
| `Shift`     | Move camera faster               |
| `I/K`       | Move car forward/backward        |
| `J/L`       | Steer car left/right             |
| `1`         | First-person camera              |
| `2`         | Third-person camera              |
| `ESC`       | Quit program                     |

## Models and Textures

### Models (in `Models/`)
- `Bird.obj`
- `part.obj` (hills)
- `Light Pole.obj`
- `generic medium.obj` (grandstand)

### Textures (in `Textures/`)
- `grass.jpg`
- `asphalt.jpg`
- `curb.jpg`
- `cobblestone.jpg`
- `moutain.jpg`
- `car_wrap.jpg`
- `tires.jpg`
- `01.png`, `02.png`, `03.png` (clouds)
- `generic medium_01_a.png`, `b.png`, `c.png` (grandstand textures)

## Dependencies

- GLEW
- GLFW
- GLM
- ASSIMP
- stb_image

Make sure all libraries are installed and properly linked.
