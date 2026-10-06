#include "arg_parser.hpp"
#include <exception>
#include <iostream>
#include <string>

constexpr const char *usage = "Usage: \ng++ [options] file...\n";

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    using arg_parser::ArgType;
    arg_parser::ArgParser parser("g++", usage);

    // or set the program name by set_xxx function
    // parser.set_program_name("cpp");
    // parser.set_usage(usage);

    // step 1: set the required arguments
    // [ArgType::Option] and [ArgType::Flag] support
    // default Flag
    // For Flag, default value will be false, no need to set manually
    parser.add_argument("verbose", "v")
        .set_description("if true, print with verbose output");

    parser.add_argument("output", "o", ArgType::Option)
        .set_description("Place the output into <file>.")
        .set_required(true);

    parser.add_argument("help", "h").set_description("print help manual");

    try {
        // step 2: parse arguments directly
        // WARN: Don't use the vector version unless you know what you're doing
        parser.parse(argc, argv);

        // step 3: traverse the positional args
        // fetch option value with type assign
        for (size_t i = 0; i < parser.size(); i++) {
            std::cout << i << " pos: " << parser.at(i) << '\n';
        }
        std::cout << "verbose: " << parser.get<bool>("v") << '\n';
        std::cout << "output: " << parser.get<std::string>("o") << '\n';

    } catch (const std::exception &err) {
        std::cerr << err.what() << '\n';
        // if you have setted usage, just leave it empty
        std::cout << parser.help();
        return 1;
    }
}
