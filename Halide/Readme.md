Wallpaper Transformer Halide Version
====================================

Wallpaper Transformer is a tool that transforms applies different effects to images/wallpapers efficiently.
The tool was originally inspired by [arch-pepe-maker](https://gitlab.com/SillyPill/arch-pape-maker)
The primary motivation to reimplement arch-pepe-maker natively is to, increase performance, allow execution on the gpu, and add new features. 

Halide was the project used to enable great performance, gpu execution (cura, opencl) and parallel execution. I managed to get performance x4 faster then the ImageMagic for blur. 

## Building

You need to have boost installed to build this. I am using the static libraries, so the binaries can be distributed easily.

1. `make halide` to download halide
2. `make` to generate optimized code
3. `make wallpaper` to build the frontend

## Usage


```
Usage: wallpaper_cpu input-file input-logo
Wallpaper configurator:
  -d [ --darken ] arg (=-0.0399999991)  Logo darken, accepts negative values for brighten
  --input-file arg                      input image file
  --input-logo arg                      logo image file
  -o [ --output-file ] arg (=wallpaper.png)
                                        output file
  -h [ --help ]                         Help
```

You can substitute the cpu part for the other version if you like. 

This is an example usage.

`./wallpaper_cpu input.png logo.png -o result.png`
