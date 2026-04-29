# MiniCycles

A Pathtracer made with C and OpenGL Compute Shader, inspired by Blender's Cycles. It uses the montecarlo path tracing algorithm for better simulating of the lighting physics and ray bounces.

This program was initially created by using the MiniRT project from 42 School as a reference, but there are some conceptual changes that I made to support different types of objects, lights and scenes (input files may differ).

Currently there are some architeractural and performance issues that needs to be addressed. The Kernel currently has a lot of branch divergent code, making poor use of the SIMT execution model (this is more obvious when we are rendering a heavy scene like bistro), for this reason I will be migrating from the current megakernel approach into a wavefront architecture in the future.

<img width="1872" height="776" alt="opa" src="https://github.com/user-attachments/assets/3a897d05-1624-480a-9861-37d8aca07537" />

##### The scene above can be found at my other repository with more complex scenes: [miniCycles Scenes](https://github.com/FelBenini/miniCycles-scenes).

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
