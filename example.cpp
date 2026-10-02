#include "arg_parser.hpp"
#include <iostream>

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    using arg_parser::ArgType;
    arg_parser::ArgParser parser;
    parser.set_program_name("example");
    parser.add_argument("input").set_default("input.png").set_description("Input file");
    parser.add_argument("output", "o", ArgType::Option).set_default("output.txt");
    parser.add_argument("count", "c", ArgType::Option).set_default("80");
    parser.add_argument("verbose", "v", ArgType::Flag).set_default("false");
    parser.add_argument("help", "h", ArgType::Flag).set_description("Show help");

    try {
        parser.parse(argc, argv);
        if (parser.has("help")) { parser.help(); return 0; }
        std::cout << parser.get<std::string>("input") << '\n'
                  << parser.get<std::string>("output") << '\n'
                  << parser.get<int>("count") << '\n'
                  << parser.get<bool>("verbose") << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        parser.help();
        return 1;
    }
    return 0;
}
