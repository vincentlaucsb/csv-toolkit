#include <cxxopts.hpp>
#include <iostream>
#include <fstream>
#include "csv_postgres.hpp"

int main(int argc, char** argv) {
    using namespace toolkit;

    cxxopts::Options options(argv[0], "Create a PostgreSQL dump file");
    options.positional_help("[in] [out]");
    options.add_options("required")
        ("input", "input file", cxxopts::value<std::string>())
        ("output", "output file", cxxopts::value<std::string>());
    options.add_options("optional")
        ("n,skiplines", "Skip the first n lines", cxxopts::value<size_t>()->default_value("0"));
    options.parse_positional({ "input", "output" });

    if (argc < 3) {
        std::cout << options.help({ "optional" }) << std::endl;
        exit(1);
    }

    try {
        auto results = options.parse(argc, argv);

        PGOptions pg_options;
        pg_options.skiplines = results["skiplines"].as<size_t>();

        std::ofstream out(argv[2]);
        toolkit::csv_to_postgres(argv[1], out, pg_options);
    }
    catch (std::runtime_error& err) {
        std::cout << "Error: " << err.what() << std::endl;
    }

    return 0;
}