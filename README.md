# MiniCycles

A Pathtracer made with C and OpenGL Compute Shader, inspired by the Blender's cycles render mechanism. It uses the montecarlo path tracing algorithm for better simulating the lighting physics and ray bounces.

<img width="1281" height="654" alt="Screenshot from 2026-04-02 19-26-12" src="https://github.com/user-attachments/assets/8b8aa97d-7bae-4df3-af38-05a477500bbf" />

## Installing, compiling and running.

```bash
git clone https://github.com/FelBenini/mini-cycles.git
cd mini-cycles
make
./cycles ./scenes/empty.rt
```

## Requirements

- GLFW installed
- GNU C Compiler
- And that's all folks

## Features

### Tonemapping

Tonemap information can be passed as a .cube file. LUT files are located at assets/lut

##### ./scenes/lucy.rt
<img width="1866" height="1012" alt="regular" src="https://github.com/user-attachments/assets/f9eeb5fd-18f1-4dc2-bed1-7d2933b41bd3" />


To tonemap the rendered scene, you can pass the LUT information on the scene file, like in the examples below.
```bash
// example_scene.rt
...
pl  0.0,0.0,0.0   0.000,0.000,0.000   15,15,15   1000,1000 0.01
lut path/to/cube_file
```

## Examples
##### ./scenes/room.rt
<img width="1916" height="1006" alt="Screenshot from 2026-04-02 19-32-13" src="https://github.com/user-attachments/assets/d360cc33-b632-44e5-a386-7a9e64306c38" />

##### ./scenes/dragon.rt
<img width="1281" height="654" alt="Screenshot from 2026-04-02 19-26-12" src="https://github.com/user-attachments/assets/8b8aa97d-7bae-4df3-af38-05a477500bbf" />
