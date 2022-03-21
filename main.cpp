// Halide tutorial lesson 7: Multi-stage pipelines

// On linux, you can compile and run it like so:
// g++ lesson_07*.cpp -g -std=c++11 -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o lesson_07
// LD_LIBRARY_PATH=../bin ./lesson_07

// On os x:
// g++ lesson_07*.cpp -g -std=c++11 -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o lesson_07
// DYLD_LIBRARY_PATH=../bin ./lesson_07

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_07_multi_stage_pipelines
// in a shell with the current directory at the top of the halide
// source tree.

#include "Halide.h"
#include <stdio.h>
#include <dlfcn.h>

using namespace Halide;

// Support code for loading pngs.
#include "halide_image_io.h"
using namespace Halide::Tools;
Var x("x"), y("y"), c("c"), xo("x_o"), yo("y_o"), yi("y_in"), xi("x_in");
// Var xo, yo, co, xi, yi, ci;

template <typename V, typename... T>
constexpr auto array_of(T &&... t)
    -> std::array<V, sizeof...(T)>
{
     return {{std::forward<T>(t)...}};
}

const auto kernel = array_of<float>(
    //     0.032439f,
    //     0.039933f,
    //     0.047815f,
    //     0.055688f,
    //     0.063085f,
    //     0.069511f,
    //     0.074497f,
    //     0.07766f,
    //     0.078743f,
    //     0.07766f,
    //     0.074497f,
    //     0.069511f,
    //     0.063085f,
    //     0.055688f,
    //     0.047815f,
    //     0.039933f,
    //     0.032439f
    0.097316f,
    0.107228f,
    0.114921f,
    0.119799f,
    0.121471f,
    0.119799f,
    0.114921f,
    0.107228f,
    0.097316f);

class WallpaperPipeline
{
public:
     const int size = (kernel.size() - 1) / 2;

     Func input_f, logo_f, blur_x, blur_y, output, blur;

     Target target;
     ImageParam input;
     ImageParam logo;

     RDom rx;
     RDom ry;

     WallpaperPipeline(ImageParam input, ImageParam logo, Target target) : target(target), input(input), logo(logo), rx(1, input.width() - 1), ry(1, input.height() - 1)
     {

          Expr clamped_x = clamp(x, 0, input.width() - 1);
          Expr clamped_y = clamp(y, 0, input.height() - 1);

          input_f(x, y, c) = ((cast<float>(input(clamped_x, clamped_y, c)) / 255.0f) - 0.5f);
          logo_f(x, y) = logo(clamped_x, clamped_y);

          // blur(x, y, c) = input_f(x, y, c);

          Expr blur_x_exp = 0;

          for (int i = -size; i <= size; i++)
          {
               blur_x_exp = blur_x_exp + kernel[size + i] * input_f(x + i, y, c);
          }

          blur_x(x, y, c) = blur_x_exp;

          Expr blur_y_exp = 0;

          for (int i = -size; i <= size; i++)
          {
               blur_y_exp = blur_y_exp + kernel[size + i] * blur_x(x, y + i, c);
          }

          blur_y(x, y, c) = blur_y_exp;

          const float darken = -.4f;
          output(x, y, c) =
              Halide::Internal::Select::make(logo(x, y) < 128,
                                             cast<uint8_t>(min(((blur_y(x, y, c) * (1.0f - darken)) + (.5f - darken / 2)), 1.0f) * 255.0f),
                                             input(x, y, c));

          // output(x, y, c) =
          //     (logo(x, y) / 255) * cast<uint8_t>(min(((blur_y(x, y, c) * (1.0f - darken)) + (.5f - darken / 2)), 1.0f) * 255.0f) +
          //     (1 - (logo(x, y) / 255)) * input(x, y, c);
     }

     void cpu()
     {
          blur.reorder(c, y, x)
              .bound(c, 0, 3)
              .unroll(c);

          // blur.update(1)
          //     .reorder(c, rx, y)
          //     .unroll(c)
          //     .split(y, yo, yi, 64)
          //     .parallel(yo, 4);
          //     .vectorize(rx, 4);

          blur.update(0)
              .reorder(c, ry, x)
              .unroll(c)
              .split(x, xo, xi, 64)
              .parallel(xo, 4);
          //     .vectorize(ry, 4);

          blur.compute_root();

          output.reorder(c, x, y).bound(c, 0, 3).unroll(c);
          // .split(x, xo, xi, 64).parallel(xo).vectorize(y, 4);

          output.compile_to_c("code/archinizator_cpu.c", {input, logo}, "generate", target);
          output.compile_to_object("lib/archinizator_cpu.o", {input, logo}, "generate", target);

          output.compile_to_lowered_stmt("out/archinizator_cpu.html", {input, logo}, HTML, target);
     }

     void gpu()
     {
          const int t = 16, g = 8;

          blur_x.compute_at(output, x).gpu_threads(x, y);
          output.gpu_tile(x, y, xi, yi, t, g);

          // output.compile_to_c("code/archinizator_gpu.c", {input, logo}, "generate", target);
          output.compile_to_object("lib/archinizator_gpu_cl.o", {input, logo}, "generate", target);

          output.compile_to_lowered_stmt("out/archinizator_gpu_cl.html", {input, logo}, HTML, target);

          output.bound(c, 0, 3).glsl(x, y, c);

          auto targetNoCL = target.without_feature(Target::OpenCL);
          targetNoCL.set_feature(Target::OpenGL);
          targetNoCL.set_feature(Target::EGL);
          targetNoCL.set_feature(Target::OpenGLCompute);

          output.output_buffer().dim(2).set_bounds(0, 3);
          output.bound(x, 0, 1920).bound(y, 0, 1080).glsl(x, y, c);

          output.compile_to_file("test_glsl",  {input, logo}, "test_glsl", targetNoCL);

          targetNoCL.set_feature(Target::CUDA);

          output.compile_to_object("lib/archinizator_gpu_cuda.o", {input, logo}, "generate", targetNoCL);

          output.compile_to_lowered_stmt("out/archinizator_gpu_cuda.html", {input, logo}, HTML, targetNoCL);
     }
};

int main(int argc, char **argv)
{
     ImageParam input(type_of<uint8_t>(), 3);
     auto d = input.dim(2);
     ImageParam logo(type_of<uint8_t>(), 2);

     Target target = get_host_target();
     // target.set_feature(Target::OpenGLCompute);
     // target.set_feature(Target::Debug);
     // target.set_feature(Target::CLHalf);
     target.set_feature(Target::OpenCL);
     target.set_feature(Target::AVX);
     // target.set_feature(Target::AVX512);
     target.set_feature(Target::EnableLLVMLoopOpt);
     target.set_feature(Target::F16C);

     WallpaperPipeline gpu(input, logo, target);
     WallpaperPipeline cpu(input, logo, target);

     gpu.gpu();
     printf("GPU compiled\n");
     // cpu.cpu();
     printf("CPU compiled\n");

     cpu.output.compile_to_header("include/archinizator.h", {input, logo}, "generate", target);

     printf("Success!\n");
     return 0;
}
