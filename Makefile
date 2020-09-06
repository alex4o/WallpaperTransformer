
all:
	g++ main.cpp -g -std=c++11 -I halide/include -I halide/tools -L halide/bin -I/usr/include/libpng16 -L/usr/lib -lpng16 -ljpeg -lHalide -lpthread -ldl -o ./bin/generate
	LD_LIBRARY_PATH=halide/bin ./bin/generate

wallpaper: cpu gpu_cuda gpu_cl

cpu:
	g++ run.cpp ./lib/archinizator_cpu.o /lib/libboost_program_options.a -O3  -g -std=c++11 -I ./include -I halide/include -I halide/tools -L halide/bin -I/usr/include/libpng16 -L/usr/lib -lpng16 -ljpeg -lpthread -ldl -o ./bin/wallpaper_cpu

gpu_cl:
	g++ run.cpp ./lib/archinizator_gpu_cl.o /lib/libboost_program_options.a -O3 -g -std=c++11 -I ./include -I halide/include -I halide/tools -L halide/bin -I/usr/include/libpng16 -L/usr/lib -lpng16 -ljpeg -lpthread -ldl -o ./bin/wallpaper_gpu_cl

gpu_cuda:
	g++ run.cpp ./lib/archinizator_gpu_cuda.o /lib/libboost_program_options.a -O3 -g -std=c++11 -I ./include -I halide/include -I halide/tools -L halide/bin -I/usr/include/libpng16 -L/usr/lib -lpng16 -ljpeg -lpthread -ldl -o ./bin/wallpaper_gpu_cuda

#gpu_gl:
#	g++ run.cpp ./lib/archinizator_gpu_opengl.o /lib/libboost_program_options.a -O3 -g -std=c++11 -I ./include -I halide/include -I halide/tools -L halide/bin -I/usr/include/libpng16 -L/usr/lib -lpng16 -ljpeg -lpthread -ldl -o ./bin/wallpaper_gpu_opengl


halide:
	wget https://github.com/halide/Halide/releases/download/v8.0.0/halide-linux-64-gcc53-800-65c26cba6a3eca2d08a0bccf113ca28746012cc3.tgz
	tar -xzf halide-linux-64-gcc53-800-65c26cba6a3eca2d08a0bccf113ca28746012cc3.tgz
	rm halide-linux-64-gcc53-800-65c26cba6a3eca2d08a0bccf113ca28746012cc3.tgz


run:
	./wallpaper

.PHONY: run, all
