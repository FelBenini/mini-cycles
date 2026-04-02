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

Currently the AGX tonemapping is procedurally done at the fragment shader, but you can also pass LUTs in the .cube format for better color controling. Some example .cube files are located at ´assets/lut´

##### ./scenes/suzanne.rt
<img width="1958" height="769" alt="AGX_comparison" src="https://github.com/user-attachments/assets/370f61a8-69ed-4afc-856b-36e0fe50fb0d" />

##### ./scenes/shapes.rt
<img width="1332" height="1740" alt="tonemap comparison" src="https://github.com/user-attachments/assets/d8cf6bbe-b54b-46e7-8f41-c2eab43eee84" />


To tonemap the rendered scene, you can pass the LUT information by command line, like the examples below.
```bash
./cycles ./path/to/scene --tonemap=cube --lut=./path/to/cube-lut # Custom LUT in .cube format
./cycles ./path/to/scene --tonemap=agx # AGX tonemapping
./cycles ./path/to/scene # No tonemapping
```

## Examples
##### ./scenes/room.rt
<img width="1916" height="1006" alt="Screenshot from 2026-04-02 19-32-13" src="https://github.com/user-attachments/assets/d360cc33-b632-44e5-a386-7a9e64306c38" />

##### ./scenes/dragon.rt
<img width="1281" height="654" alt="Screenshot from 2026-04-02 19-26-12" src="https://github.com/user-attachments/assets/8b8aa97d-7bae-4df3-af38-05a477500bbf" />
