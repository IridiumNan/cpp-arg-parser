#include "arg_parser.hpp"

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void requireAt(bool ok, int line) {
    if (!ok) {
        std::fprintf(stderr, "argument parser assertion failed at line %d\n", line);
        throw std::runtime_error("Assertion failed at line " + std::to_string(line));
    }
}
#define require(ok) requireAt((ok), __LINE__)

template <typename F>
void requireInvalidArgument(F action, const std::string& message) {
    try { action(); }
    catch (const std::invalid_argument& error) {
        if (std::string(error.what()) != message) {
            throw std::runtime_error("Expected error [" + message + "] but got [" + error.what() + "]");
        }
        return;
    }
    throw std::runtime_error("Expected std::invalid_argument: " + message);
}

void testBasicBehavior() {
    using namespace arg_parser;
    {
        ArgParser parser;
        parser.add_argument("input").set_required(true);
        parser.add_argument("output", "o", ArgType::Option);
        parser.add_argument("columns", std::nullopt, ArgType::Option).set_default("200");
        parser.add_argument("help", "h", ArgType::Flag);
        parser.parse({"photo.png", "-o", "out.txt", "-h"});
        require(parser.has("input") && parser.get<std::string>("input") == "photo.png");
        require(parser.has("output") && parser.get<std::string>("output") == "out.txt");
        require(parser.has("help") && parser.get<std::string>("help") == "true");
        require(!parser.has("columns") && parser.get<int>("columns") == 200);
        require(!parser.has("unknown"));
        requireInvalidArgument([&] { parser.get<std::string>("unknown"); }, "Argument not registered: unknown");
    }
    // Defaults do not count as explicitly provided arguments.
    {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option).set_default("default.png");
        parser.add_argument("font", std::nullopt, ArgType::Option);
        parser.parse(std::vector<std::string>{});
        require(!parser.has("output") && parser.get<std::string>("output") == "default.png");
        requireInvalidArgument([&] { parser.get<std::string>("font"); }, "Argument not provided: font");
    }
    for (const auto& args : std::vector<std::vector<std::string>>{
        {"--output=dir/a=b.png"}, {"-o=dir/a=b.png"},
        {"--output", "dir/a=b.png"}, {"-o", "dir/a=b.png"}}) {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        parser.parse(args);
        require(parser.has("output") && parser.get<std::string>("output") == "dir/a=b.png");
    }
    // Explicit empty option values are rejected, even when a default exists.
    for (const auto& args : std::vector<std::vector<std::string>>{
        {"--output="}, {"-o", ""}}) {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option).set_default("default.png");
        requireInvalidArgument([&] { parser.parse(args); },
                               "Missing value for argument: " + args.front());
    }
    {
        ArgParser parser;
        parser.add_argument("first");
        parser.add_argument("second");
        parser.add_argument("third");
        parser.parse({"", "--", "-photo.png", "--"});
        require(parser.has("first") && parser.get<std::string>("first").empty());
        require(parser.get<std::string>("second") == "-photo.png");
        require(parser.get<std::string>("third") == "--");
    }
    {
        ArgParser parser;
        parser.add_argument("count", "c", ArgType::Option);
        parser.add_argument("ratio", std::nullopt, ArgType::Option);
        parser.parse({"-c=-12", "--ratio=1.5"});
        require(parser.get<int>("count") == -12);
        require(parser.get<double>("ratio") == 1.5);
    }
    for (const auto& value : {"12x", "not-a-number", "999999999999999999999999"}) {
        ArgParser parser;
        parser.add_argument("count", std::nullopt, ArgType::Option);
        parser.parse({"--count", value});
        const std::string expected = std::string(value) == "12x"
            ? "Invalid value for argument 'count': '12x'"
            : "Failed to convert argument 'count' with value '" + std::string(value) + "'";
        requireInvalidArgument([&] { parser.get<int>("count"); }, expected);
    }
    {
        ArgParser parser;
        requireInvalidArgument([&] { parser.add_argument(""); }, "Argument name must not be empty");
        parser.add_argument("input");
        requireInvalidArgument([&] { parser.add_argument("input"); }, "Argument already registered: input");
    }
    {
        ArgParser parser;
        requireInvalidArgument([&] { parser.parse({"--unknown"}); }, "Unrecognized argument: --unknown");
    }
    {
        ArgParser parser;
        parser.add_argument("input");
        requireInvalidArgument([&] { parser.parse({"a", "b"}); }, "Unexpected positional argument: b");
    }
    {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        requireInvalidArgument([&] { parser.parse({"-o"}); }, "Missing value for argument: -o");
    }
    {
        ArgParser parser;
        parser.add_argument("input").set_default("default.png").set_required(true);
        requireInvalidArgument([&] { parser.parse(std::vector<std::string>{}); }, "Required argument not provided: input");
    }
    // Native argv normalization skips the executable and preserves Unicode.
    {
        ArgParser parser;
        parser.add_argument("input");
#ifdef _WIN32
        wchar_t program[] = L"arg_parser";
        wchar_t input[] = L"\u56fe\u7247.png";
        wchar_t* argv[] = {program, input};
#else
        char program[] = "arg_parser";
        char input[] = "\xe5\x9b\xbe\xe7\x89\x87.png";
        char* argv[] = {program, input};
#endif
        parser.parse(2, argv);
        require(parser.get<std::string>("input") == "\xe5\x9b\xbe\xe7\x89\x87.png");
    }
    {
        ArgParser parser;
        parser.set_program_name("arg_parser");
        parser.set_note("Example usage");
        parser.add_argument("input").set_description("Input file");
        parser.add_argument("output", "o", ArgType::Option).set_description("Output file");
        parser.add_argument("help", "h", ArgType::Flag);
        std::ostringstream output;
        auto* previous = std::cout.rdbuf(output.rdbuf());
        parser.help();
        std::cout.rdbuf(previous);
        require(output.str().find("Usage: arg_parser [options] <input>") != std::string::npos);
        require(output.str().find("--output, -o <value>") != std::string::npos);
        require(output.str().find("Output file") != std::string::npos);
        require(output.str().find("Example usage") != std::string::npos);
    }
}

// Registration conflicts have no established diagnostic wording: require
// invalid_argument and a nonempty diagnostic instead of inventing wording.
template <typename F>
void requireRejected(F action) {
    try { action(); }
    catch (const std::invalid_argument& error) {
        require(std::string(error.what()).size() > 0);
        return;
    }
    throw std::runtime_error("Expected std::invalid_argument, but operation succeeded");
}

int main() {
    using namespace arg_parser;
    int failures = 0;
    int total = 0;
    auto run = [&](const std::string& name, const std::function<void()>& test) {
        ++total;
        try {
            test();
            std::printf("PASS %s\n", name.c_str());
        } catch (const std::exception& error) {
            ++failures;
            std::fprintf(stderr, "FAIL %s: %s\n", name.c_str(), error.what());
        } catch (...) {
            ++failures;
            std::fprintf(stderr, "FAIL %s: unexpected exception type\n", name.c_str());
        }
    };
    run("basic behavior and exact diagnostics", testBasicBehavior);
    run("has supports canonical names and aliases", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        parser.parse({"-o=result.png"});
        require(parser.has("output") && parser.has("o"));
    });
    run("canonical get after alias input", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option).set_default("default.png");
        parser.parse({"-o=result.png"});
        require(parser.get<std::string>("output") == "result.png");
    });
    run("canonical typed get after alias input", [] {
        ArgParser parser;
        parser.add_argument("count", "c", ArgType::Option).set_default("200");
        parser.parse({"-c=12"});
        require(parser.get<int>("count") == 12);
    });
    run("duplicate alias is rejected", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        requireRejected([&] { parser.add_argument("other", "o", ArgType::Option); });
        parser.parse({"-o=result.png"});
        require(parser.get<std::string>("output") == "result.png");
    });
    run("alias cannot shadow a canonical name", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        requireRejected([&] { parser.add_argument("other", "output", ArgType::Option); });
    });
    run("canonical name cannot shadow an alias", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        requireInvalidArgument([&] { parser.add_argument("o"); }, "Argument already registered: o");
    });
    for (const auto& args : std::vector<std::vector<std::string>>{
        {"--output=a.png", "--output=b.png"}, {"--output=a.png", "-o=b.png"},
        {"-o=a.png", "--output=b.png"}, {"-o=a.png", "-o=b.png"}}) {
        run("duplicate option " + args.front() + " then " + args.back(), [args] {
            ArgParser parser;
            parser.add_argument("output", "o", ArgType::Option);
            requireInvalidArgument([&] { parser.parse(args); },
                                   "Argument already provided: " + args.back());
        });
    }
    run("duplicate flag through alias is rejected", [] {
        ArgParser parser;
        parser.add_argument("help", "h", ArgType::Flag);
        requireInvalidArgument([&] { parser.parse({"--help", "-h"}); },
                               "Argument already provided: -h");
    });
    for (const auto& token : {"--help=false", "--help="}) {
        run(std::string("flag rejects value ") + token, [token] {
            ArgParser parser;
            parser.add_argument("help", "h", ArgType::Flag);
            requireInvalidArgument([&] { parser.parse({token}); },
                                   "Flag argument cannot have a value: " + std::string(token));
        });
    }
    run("registered option is not consumed as a missing value", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        parser.add_argument("help", "h", ArgType::Flag);
        requireInvalidArgument([&] { parser.parse({"--output", "--help"}); },
                               "Missing value for argument: --output");
    });
    run("inline dashed value is accepted", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option);
        parser.parse({"--output=-result.png"});
        require(parser.get<std::string>("output") == "-result.png");
    });
    run("same-name alias does not bypass duplicate registration", [] {
        ArgParser parser;
        parser.add_argument("output", std::nullopt, ArgType::Option);
        requireInvalidArgument([&] { parser.add_argument("output", "output", ArgType::Option); },
                               "Argument already registered: output");
    });
    run("fresh argument can use its own name as alias", [] {
        ArgParser parser;
        parser.add_argument("output", "output", ArgType::Option);
        parser.parse({"--output=result.png"});
        require(parser.has("output") && parser.get<std::string>("output") == "result.png");
    });
    // The supported lifetime is one parse call per parser instance.
    for (const auto& args : std::vector<std::vector<std::string>>{
        {"--output"}, {"--output="}, {"--output", ""}, {"--output", "--help"},
        {"--output", "--"}, {"--output", "-12"}, {"--output", "-file.png"}}) {
        run("missing option value: " + (args.size() == 2 ? args.back() : args.front()), [args] {
            ArgParser parser;
            parser.add_argument("output", "o", ArgType::Option).set_default("default.png");
            parser.add_argument("help", "h", ArgType::Flag);
            requireInvalidArgument([&] { parser.parse(args); },
                                   "Missing value for argument: " + args.front());
            require(!parser.has("output"));
            require(parser.get<std::string>("output") == "default.png");
        });
    }
    for (const auto& token : {"--input=photo.png", "--input", "-i=photo.png"}) {
        run(std::string("positional rejects option syntax: ") + token, [token] {
            ArgParser parser;
            parser.add_argument("input", "i");
            requireInvalidArgument([&] { parser.parse({token}); },
                "Positional argument cannot be provided as an option: " + std::string(token));
            require(!parser.has("input"));
        });
    }
    for (const auto& token : {"--unknown", "--unknown=value", "-x", "-"}) {
        run(std::string("unknown option: ") + token, [token] {
            ArgParser parser;
            requireInvalidArgument([&] { parser.parse({token}); },
                                   "Unrecognized argument: " + std::string(token));
        });
    }
    run("Flag bool and string retrieval", [] {
        ArgParser parser;
        parser.add_argument("help", "h", ArgType::Flag);
        parser.parse({"-h"});
        require(parser.has("help") && parser.has("h"));
        require(parser.get<bool>("help"));
        require(parser.get<std::string>("help") == "true");
    });
    run("bool defaults do not mark arguments as provided", [] {
        ArgParser parser;
        parser.add_argument("enabled", std::nullopt, ArgType::Flag).set_default("true");
        parser.add_argument("disabled", std::nullopt, ArgType::Flag).set_default("false");
        parser.parse(std::vector<std::string>{});
        require(!parser.has("enabled") && parser.get<bool>("enabled"));
        require(!parser.has("disabled") && !parser.get<bool>("disabled"));
    });
    run("missing bool has an exact diagnostic", [] {
        ArgParser parser;
        parser.add_argument("help", "h", ArgType::Flag);
        parser.parse(std::vector<std::string>{});
        requireInvalidArgument([&] { parser.get<bool>("help"); }, "Argument not provided: help");
        requireInvalidArgument([&] { parser.get<bool>("unknown"); }, "Argument not registered: unknown");
    });
    run("string specialization preserves spaces", [] {
        ArgParser parser;
        parser.add_argument("font", std::nullopt, ArgType::Option);
        parser.parse({"--font", " custom font.ttf "});
        require(parser.get<std::string>("font") == " custom font.ttf ");
    });
    run("empty positional default stays absent", [] {
        ArgParser parser;
        parser.add_argument("input").set_default("");
        parser.parse(std::vector<std::string>{});
        require(!parser.has("input") && parser.get<std::string>("input").empty());
    });
    run("required option is not satisfied by a default", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option).set_default("default.png").set_required(true);
        requireInvalidArgument([&] { parser.parse(std::vector<std::string>{}); },
                               "Required argument not provided: output");
    });
    run("required option accepts alias input", [] {
        ArgParser parser;
        parser.add_argument("output", "o", ArgType::Option).set_required(true);
        parser.parse({"-o=result.png"});
        require(parser.has("output") && parser.get<std::string>("output") == "result.png");
    });
    run("required Flag can be provided", [] {
        ArgParser parser;
        parser.add_argument("confirm", "y", ArgType::Flag).set_required(true);
        parser.parse({"-y"});
        require(parser.has("confirm") && parser.get<bool>("confirm"));
    });
    run("mixed positionals and options retain order", [] {
        ArgParser parser;
        parser.add_argument("first");
        parser.add_argument("second");
        parser.add_argument("count", "c", ArgType::Option);
        parser.parse({"one", "-c=12", "two"});
        require(parser.get<std::string>("first") == "one");
        require(parser.get<std::string>("second") == "two");
        require(parser.get<int>("count") == 12);
    });
    std::printf("%d/%d cases passed; %d failed\n", total - failures, total, failures);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
