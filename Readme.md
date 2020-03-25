Wallpaper Transformer
=====================

I wrote this because image magic is slow, and doesn't support running code on the gpu, without recompilation. 

This used the halide project, to generate a gaussian blur filter.
Even a naive implementation is x4 times faster then the image magic one,.
There are cuda and opencl versions too, that should work, but I haven't gotten good results out of them.

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


