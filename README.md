# Hobby Raytracer

Just a simple Ray Tracer I made initially following the [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html) course by [Peter Shirely](https://github.com/petershirley).

## Features Added
- Multithreading support
- Rough dieletric materials
- ACES Tonemapping and correct gamma correction
- Triangle intersections and polygon meshes loaded using [Assimp](https://github.com/assimp/assimp)

## Build
```batch
git clone --recurse-submodules https://github.com/Todegal/HobbyRaytracer.git
cmake -S . -B out
```

## Example Images

### Cornell Box
![Cornell Box](/sampleImages/Cornell-Box.png)

### Demonstration of infinite reflective surfaces
![Demonstration of infinite reflective surfaces](/sampleImages/Awesome-Reflections.bmp)

### The test image set out at the end of the course
![The test image set out at the end of the course](/sampleImages/Scattered-Balls.png)
