#include "csv_json.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    using namespace toolkit;

    cxxopts::Options options(argv[0], "Convert CSV to JSON");
    options.positional_help("[in] [out]");
    options.add_options("required")
        ("input", "input file", cxxopts::value<std::string>())
        ("output", "output file", cxxopts::value<std::string>());
    options.parse_positional({ "input", "output" });

    if (argc < 3) {
        std::cout << options.help({ "optional" }) << std::endl;
        exit(1);
    }

    try {
        auto results = options.parse(argc, argv);

        std::ofstream out(argv[2]);
        toolkit::csv_to_json(argv[1], out);
    }
    catch (std::runtime_error& err) {
        std::cout << "Error: " << err.what() << std::endl;
    }

    return 0;
}