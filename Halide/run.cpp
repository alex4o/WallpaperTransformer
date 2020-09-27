#include "archinizator.h"
// We want to continue to use our Halide::Buffer with AOT-compiled
// code, so we explicitly include it. It's a header-only class, and
// doesn't require libHalide.
#include "HalideBuffer.h"
using namespace Halide;

#include "halide_image_io.h"
using namespace Halide::Tools;

#include <boost/program_options.hpp>
#include <stdio.h>
#include <iostream>

using namespace boost::program_options;

int main(int argc, char **argv)
{

    Halide::Runtime::Buffer<uint8_t> input;
    Halide::Runtime::Buffer<uint8_t> logo;

    try
    {
        options_description desc{"Wallpaper configurator"};
        positional_options_description positional;

        desc.add_options()
        ("darken,d", value<float>()->default_value(-0.04f), "Logo darken, accepts negative values for brighten")
        ("input-file", value<std::string>(), "input file")
        ("input-logo", value<std::string>(), "logo file")
        ("output-file,o", value<std::string>()->default_value("wallpaper.png"), "output file")
        ("help,h", "Help");

        positional.add("input-file", 1);
        positional.add("input-logo", 1);

        variables_map vm;

        store(command_line_parser(argc, argv).positional(positional).options(desc).run(), vm);
        notify(vm);

        bool help = false;
        if (vm.count("input-file"))
        {
            input = load_image(vm["input-file"].as<std::string>());
        }
        else
        {
            help = true;
        }

        if (vm.count("input-logo"))
        {
            logo = load_image(vm["input-logo"].as<std::string>());
        }
        else
        {
            help = true;
        }

        if (vm.count("help") or help)
        {
            std::cout << "Usage: wallpaper";
            for (int i = 0; i < positional.max_total_count(); i++)
            {
                std::cout << " " << positional.name_for_position(i);
            }
            std::cout << std::endl;
            std::cout << desc;
            return 0;
        }

            
        Halide::Runtime::Buffer<uint8_t> output(input.width(), input.height(), 3);

        int error = generate(input, logo, output);
        if (error)
        {
            printf("Halide returned an error: %d\n", error);
            return -1;
        }

        save_image(output, vm["output-file"].as<std::string>());

        printf("Success!\n");
    }
    catch (const error &ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}