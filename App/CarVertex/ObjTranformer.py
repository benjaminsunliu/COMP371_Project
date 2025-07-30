from pathlib import Path

# Load the contents of the uploaded OBJ file
obj_path = Path(__file__).parent / "truck.obj"
obj_text = obj_path.read_text()

# Parse vertices and faces (basic parsing)
vertices = []
faces = []

for line in obj_text.splitlines():
    if line.startswith("v "):
        parts = line.strip().split()
        x, y, z = map(float, parts[1:4])
        vertices.append((x, y, z))
    elif line.startswith("f "):
        parts = line.strip().split()
        face = [int(p.split("/")[0]) - 1 for p in parts[1:]]  # OBJ index starts at 1
        if len(face) == 3:
            faces.append(face)
        elif len(face) == 4:
            # split quad into two triangles
            faces.append([face[0], face[1], face[2]])
            faces.append([face[2], face[3], face[0]])

# Build float array with gray color for each vertex in each triangle
gray = [0.7, 0.7, 0.7]
flattened = []

for face in faces:
    for idx in face:
        vx, vy, vz = vertices[idx]
        flattened.extend([vx, vy, vz] + gray)

# Format for C++ inline float array
header_lines = ["#pragma once\n", "inline float cybertruckVertices[] = {\n"]
for i in range(0, len(flattened), 6):
    x, y, z, r, g, b = flattened[i:i+6]
    header_lines.append(f"    {x:.4f}f, {y:.4f}f, {z:.4f}f,  {r:.1f}f, {g:.1f}f, {b:.1f}f,\n")
header_lines.append("};\n")

output_path = Path(__file__).parent / "CyberTruck.h"
output_path.write_text("".join(header_lines))
output_path.name
print("Done") # Need to refresh