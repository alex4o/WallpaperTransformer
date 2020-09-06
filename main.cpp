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
    0.097316f
);

class WallpaperPipeline
{
public:
     const int size = (kernel.size() - 1) / 2;

     Func input_f, logo_f, blur_x, blur_y, output;

     Target target;
     ImageParam input;
     ImageParam logo;

     WallpaperPipeline(ImageParam input, ImageParam logo, Target target) : target(target), input(input), logo(logo)
     {

          Expr clamped_x = clamp(x, 0, input.width() - 1);
          Expr clamped_y = clamp(y, 0, input.height() - 1);

          input_f(x, y, c) = ((cast<float>(input(clamped_x, clamped_y, c)) / 255.0f) - 0.5f);
          logo_f(x, y) = logo(clamped_x, clamped_y);

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
          blur_x.reorder(c, x, y)
              .bound(c, 0, 3)
              .vectorize(c)
			  .split(x, xo, xi, 8)
			  .unroll(xi, 4)
			  .parallel(y);

          blur_x.compute_root();
          output.reorder(c, x, y).bound(c, 0, 3).vectorize(c).split(x, xo, xi, 64).parallel(xo).parallel(y).unroll(xi, 4);

          output.compile_to_c("code/archinizator_cpu.c", {input, logo}, "generate", target);
          output.compile_to_object("lib/archinizator_cpu.o", {input, logo}, "generate", target);

          output.compile_to_lowered_stmt("out/archinizator_cpu.html", {input, logo}, HTML, target);
     }

     void gpu()
     {

          blur_x.reorder(c, y, x).bound(c, 0, 3).vectorize(c).compute_root();

          output.reorder(c, x, y).bound(c, 0, 3).vectorize(c).gpu_tile(x, y, xo, yo, xi, yi, 16, 21);

          // output.compile_to_c("code/archinizator_gpu.c", {input, logo}, "generate", target);
          output.compile_to_object("lib/archinizator_gpu_cl.o", {input, logo}, "generate", target);

          output.compile_to_lowered_stmt("out/archinizator_gpu_cl.html", {input, logo}, HTML, target);

          auto targetNoCL = target.without_feature(Target::OpenCL);
          targetNoCL.set_feature(Target::CUDA);
          targetNoCL.set_feature(Target::CUDACapability30);

          output.compile_to_object("lib/archinizator_gpu_cuda.o", {input, logo}, "generate", targetNoCL);

          output.compile_to_lowered_stmt("out/archinizator_gpu_cuda.html", {input, logo}, HTML, targetNoCL);
     }
};

int main(int argc, char **argv)
{
     ImageParam input(type_of<uint8_t>(), 3);
     ImageParam logo(type_of<uint8_t>(), 2);

     Target target = get_host_target();
     // target.set_feature(Target::OpenGLCompute);
     // target.set_feature(Target::Debug);
     // target.set_feature(Target::CLHalf);
     target.set_feature(Target::OpenCL);
     target.set_feature(Target::AVX);
     target.set_feature(Target::SSE41);
     target.set_feature(Target::AVX2);
     target.set_feature(Target::EnableLLVMLoopOpt);
     target.set_feature(Target::F16C);

     WallpaperPipeline gpu(input, logo, target);
     WallpaperPipeline cpu(input, logo, target);

     gpu.gpu();
     cpu.cpu();

     cpu.output.compile_to_header("include/archinizator.h", {input, logo}, "generate", target);

     printf("Success!\n");
     return 0;
}
