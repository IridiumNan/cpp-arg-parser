#include "arg_parser.hpp"
#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    using arg_parser::ArgType;
    arg_parser::ArgParser parser;
    // optional: set the program name
    // parser.set_program_name("example");

    // step 1: set the required arguments
    // [ArgType::Option] and [ArgType::Flag] support
    // default Flag
    parser.add_argument("verbose", "v")
        .set_default("false")
        .set_description("if true, print with verbose output");

    parser.add_argument("output", "o", ArgType::Option)
        .set_description("Place the output into <file>.");

    std::string usage("Usage: g++ [options] file...");
    try {
        // step 2: parse arguments directly
        // WARN: Don't use the vector version unless you know what you're doing
        parser.parse(argc, argv);

        // step 3: traverse the positional args
        // fetch option value with type assign
        for (size_t i = 0; i < parser.size(); i++) {
            std::cout << i + 1 << " pos: " << parser.at(i) << '\n';
        }
        std::cout << "verbose: " << parser.get<bool>("v") << '\n';
        std::cout << "output: " << parser.get<std::string>("o") << '\n';

    } catch (const std::exception &err) {
        std::cerr << err.what() << '\n';
        std::cout << parser.help(usage);
        return 1;
    }
}
