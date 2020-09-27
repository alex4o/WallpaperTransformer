Wallpaper Transformer
=====================

This are some experiments trying to see, how GPU compute can be done and how fast it is. 
The experiments ware originaly and unironically inspired by [arch pepe maker](https://gitlab.com/SillyPill/arch-pape-maker/-/tree/master). It uses the gausian blur filter from Image Magic, which takes a second to complete processing for a single image. As implementing that is not very complicated that got me thinking, can I and how to make this faster without much effort. Around the same time I found aut about [Halide](https://halide-lang.org/) and used it to reimplemnt it. The results from that can be seen in the Halide folder, but TLDR Halide's CPU implementation was 4 times faster then the ImageMagic one. However the GPU implementation wasn't faster then the CPU one. For that precise reason I decided on implementing a gausian blur filter using only OpenCL. The results from that can be seen in the PyOpenCL folder. But TLDR the pure OpenCL implementation uses texture memmory which is way faster to read/write then generic OpenCL buffers(Halide con use only them) and it is fast enough to run realtime. This made me curious how fast can this be done using vulkan (Not there yet).

Todo:
[] Add benchmark numbers
[] Make the python version standalone with a nice command lline argument interface
[] Explore doing the same thing with vulkan
