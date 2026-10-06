#include "arg_parser.hpp"
#include <exception>
#include <iostream>
#include <string>

typedef size_t IDX;

// you can use the constexpr
// constexpr IDX IDX_SRC = 0;
// constexpr IDX IDX_DST = 1;

// highly recommend enum
enum Pos : IDX { SRC = 0, DST = 1 };

constexpr const char *usage = "Usage: cp src dst\n";

void do_copy_dir(const std::string &src, const std::string &dst) {
    std::cout << "copying dir from " << src << " to " << dst << '\n';
}

void do_copy(const std::string &src, const std::string &dst) {
    std::cout << "copying file " << src << " to " << dst << '\n';
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    arg_parser::ArgParser parser("cp", usage);

    // or set by set_xxx function
    // parser.set_program_name("cp");
    // parser.set_usage(usage);

    // define the flag -r
    // it's default value will be seted as false
    parser.add_argument("recursive", "r")
        .set_description("copy directories recursively");

    parser.add_argument("help", "h").set_description("print help manual");
    try {
        parser.parse(argc, argv);

    } catch (const std::exception &err) {
        std::cout << err.what() << '\n';
        // we have set the usage when creating parser
        // so let it empty
        std::cout << parser.help();
        return 1;
    }

    // resolve the help case
    if (parser.get<bool>("h")) {
        std::cout << parser.help();
        return 0;
    }

    if (parser.size() != 2) {
        std::cout << "arguments count error\n";
        std::cout << parser.help();
        return 1;
    }

    // support fetch by alias
    if (parser.get<bool>("r")) {
        do_copy_dir(parser.at(Pos::SRC), parser.at(Pos::DST));
    } else {
        do_copy(parser.at(Pos::SRC), parser.at(Pos::DST));
    }
}
