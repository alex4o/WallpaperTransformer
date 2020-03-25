Wallpaper Transformer
=====================

Wallpaper Transformer is a tool that transforms modifies wallpapers, with different effect efficiently.
The tool was originally inspired by [arch-pepe-maker](https://gitlab.com/SillyPill/arch-pape-maker)
The primary motivation for the "rewrite" was to, increase performance, allow execution on the gpu, and add new features. 

Halide was the project used to enable great performance, gpu execution (cura, opencl) and parallel execution. I managed to get performance x4 faster then the ImageMagic for blur. 

Building
========

You need to have boost installed to build this. I am using the static libraries, so the binaries can be distributed easily.

1. `make halide` to download halide
2. `make` to generate optimized code
3. `make wallpaper` to build the frontend


Usage
=====

You can substitute the cpu part for the other version if you like.
`./wallpaper_cpu input.png logo.png -o result.png`


