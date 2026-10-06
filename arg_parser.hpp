// Introduction
//
// arg_parser - A lightweight C++17 single-header command-line parser.
//
// For usage and more examples, see repository:
// https://github.com/IridiumNan/cpp-arg-parser
//
// Feedback and suggestions are welcome via GitHub Issues:
// https://github.com/IridiumNan/cpp-arg-parser/issues
//
// Based on https://github.com/KAI-SHUNG/arg_parser.
// Requires C++17. No third-party dependencies.

// LICENSE
//
// SPDX-License-Identifier: MIT
// See license full text as below.
//
// MIT License
//
// Copyright (c) 2024 KAI-SHUNG (original author)
// Copyright (c) 2026 IridiumNan (modifications)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace arg_parser {

enum class ArgType {
    // Flag is argument withou any value
    // If provided, it will be set as true, else use default value
    Flag,
    Option,
};

struct Argument {
    ArgType type;
    // name of argument, e.g port, debug, verbose
    std::string name;

    // alias is the short name of argument e.g. p, d, v
    std::optional<std::string> alias;

    // The value of this argument
    // For [ArgType::Flag] it will be true or false (string)
    // For [ArgType::Option] it can be any string
    std::optional<std::string> value;

    // description that will be printed on help manual
    std::string description;

    // if required, it will throw error if this arguments is not provided
    bool required = false;

    /**
     * @brief Set the default value for this option
     * if a [ArgType::Flag], it must be "false" or "true"
     * else it will throw an error
     * */
    Argument &set_default(const std::string &default_val);

    /**
     * @brief Set description for this argument
     * it will be used to build the help manual
     * */
    Argument &set_description(const std::string &desc);

    /**
     * Set if this argument required for program
     * if true and user not provided, it will throw error
     * [std::invalid_argument] when fetch by [ArgParser::get]
     * */
    Argument &set_required(bool req);
};

class ArgParser {
  private:
    std::vector<std::shared_ptr<Argument>> registery;

    // a map from name (and alias) to argument
    std::unordered_map<std::string, std::shared_ptr<Argument>> name_to_arg;

    // parsed store arguments that has been provided
    // including [ArgType::Flag] and [ArgType::Option]
    // key is the name of argument
    // value is the value
    std::unordered_map<std::string, std::string> parsed;

    // positional args store all arguments without register
    // You can visit all by [at] function
    std::vector<std::string> positional_args;

    // _usage store the basic usage of this program
    // set by [set_usage] function
    std::string _usage;

    std::string _program_name;

    // check if a argument has been registered by [add_argument] function
    bool is_registered(const std::string &name) const;

    // check if all required argument provided
    // if not, throw [std::invalid_argument]
    void check_required_arguments() const;

    // set the first arg (always program name) as [program_name] then drop it
    std::vector<std::string> normalize_args(int argc, char **argv);
#ifdef _WIN32
    /**
     * @brief Encode one Windows command-line argument as UTF-8.
     *
     * @param value Non-null, null-terminated UTF-16 argument.
     *
     * @return UTF-8 string without its terminating null byte.
     */
    static std::string to_utf8(const wchar_t *value);
    static std::vector<std::string> normalize_args(int argc, wchar_t **argv);
#endif

  public:
    /**
     * Create a new parser with program name and usage
     * You can set name and usage by
     * [set_program_name] and [set_usage]*/
    ArgParser(const std::optional<std::string> &program_name = std::nullopt,
              const std::optional<std::string> &usage = std::nullopt) {
        if (program_name != std::nullopt) {
            _program_name = program_name.value();
        }
        if (usage != std::nullopt) {
            _usage = usage.value();
        }
    }

    /**
     * @brief Set the program name for usage messages.
     * if not set, it will be the first argument when program exec
     */
    void set_program_name(const std::string &name) { _program_name = name; }

    /**
     * @brief Set the usage of program
     * e.g.
     * Usage: cp [OPTION]... [-T] SOURCE DEST
        or:  cp [OPTION]... SOURCE... DIRECTORY

        The options will auto generated on [help] function
    */
    void set_usage(const std::string &usage) { _usage = usage; }

    /**
     * @brief Add a new argument to the parser, suggest chain calls to set its
     * properties.
     *
     * @param name The name without leading dashes; must be unique.
     * @param alias The optional alias without leading dashes; must be unique.
     * @param type The type of the argument. default bool Flag;
     * You can also use [ArgType::Option] for arguments that require value
     * A default value for [ArgType::Flag] will be false, no need to set
     *
     * @return A reference to the newly added argument.
     */
    Argument &
    add_argument(const std::string &name,
                 const std::optional<std::string> &alias = std::nullopt,
                 ArgType type = ArgType::Flag);

    /**
     * @brief Check if an argument has been provided.
     * Both the argument name and its alias are supported.
     * @param name The name or alias of the argument to check.
     */
    bool has(const std::string &name) const;

    /**
     * @brief build help message for all registered arguments, return a string.
     * @param usage is the basic positional argument and program introduction
     * the optional the flag description are auto generated
     * If usage has been set, you don't need to provide, highly recommend set
     * usage by [set_usage]
     */
    std::string help(const std::optional<std::string> &usage) const;

    /**
     * @brief
     * Get the value of an argument.
     * If the argument is registered but not provided,
     * return the default value if set, otherwise throw an exception.
     * @param name alias name are supported.
     */
    template <typename T> T get(const std::string &name) const;

    /**
     * @brief
     * Return the positional arguments count
     * */
    size_t size() const { return positional_args.size(); }
    /**
     * @brief
     * Get positional arguments by index
     * it will not includes the program name itself (the argv[0])
     * if out of range, it will throw [std::out_of_range]
     * */
    std::string at(size_t idx) const;

    /**
     * @brief parse command-line arguments.
     * Just provide the argc and argv directly
     * Don't cut any arguments manually
     */
    void parse(int argc, char **argv);

#ifdef _WIN32
    /**
     * @brief parse command-line arguments (Windows version).
     */
    void parse(int argc, wchar_t **argv);
#endif

    /**
     * @brief parse normalized command-line arguments.
     * WARN: This function is defined for test
     * If you want to use it, you should remove the program name from vector
     * You should provide ["main.cpp", "arg.cpp", "-o", "bin/main"]
     * instead of ["g++", "main.cpp", "arg.cpp", "-o", "bin/main"]
     */
    void parse(const std::vector<std::string> &args);
};

} // namespace arg_parser

// Implementation of template methods.

template <typename T>
T arg_parser::ArgParser::get(const std::string &name) const {
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }
    // Resolve alias -> canonical name once.
    const auto arg_ptr = name_to_arg.at(name);
    const std::string &canonical = arg_ptr->name;

    std::string arg;
    auto it = parsed.find(canonical);
    if (it == parsed.end()) {
        if (arg_ptr->value.has_value()) {
            arg = arg_ptr->value.value();
        } else {
            throw std::invalid_argument("Argument not provided: " + name);
        }
    } else {
        arg = it->second;
    }
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }

    T result;
    std::stringstream ss(arg);

    if (!(ss >> result)) {
        throw std::invalid_argument("Failed to convert argument '" + name +
                                    "' with value '" + arg + "'");
    }
    // Check for any remaining characters in the stream after reading the value
    // For example, want a integer, input is "123abc" -> 123 is read, but "abc"
    // remains
    ss >> std::ws;
    if (!ss.eof()) {
        throw std::invalid_argument("Invalid value for argument '" + name +
                                    "': '" + arg + "'");
    }
    return result;
}

template <>
inline std::string
arg_parser::ArgParser::get<std::string>(const std::string &name) const {
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }

    const auto arg_ptr = name_to_arg.at(name);
    auto it = parsed.find(arg_ptr->name);
    if (it != parsed.end()) {
        return it->second;
    }
    if (arg_ptr->value.has_value()) {
        return arg_ptr->value.value();
    }
    throw std::invalid_argument("Argument not provided: " + arg_ptr->name);
}

template <>
inline bool arg_parser::ArgParser::get<bool>(const std::string &name) const {
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }
    const auto arg_ptr = name_to_arg.at(name);
    auto it = parsed.find(arg_ptr->name);
    if (it != parsed.end()) {
        return it->second == "true";
    }
    if (arg_ptr->value.has_value()) {
        return arg_ptr->value.value() == "true";
    }
    throw std::invalid_argument("Argument not provided: " + name);
}

#ifdef _WIN32
#include <windows.h>
#endif
namespace arg_parser {

/// Argument class member functions

inline Argument &Argument::set_default(const std::string &default_val) {
    if (this->type == ArgType::Flag) {
        if (default_val != "true" && default_val != "false") {
            throw std::invalid_argument(
                "A flag default value should be true or false");
        }
    }
    value = default_val;
    return *this;
}

inline Argument &Argument::set_description(const std::string &desc) {
    description = desc;
    return *this;
}

inline Argument &Argument::set_required(bool req) {
    required = req;
    return *this;
}

// ArgParser member functions

inline bool ArgParser::is_registered(const std::string &name) const {
    return name_to_arg.count(name) > 0;
}

inline void ArgParser::check_required_arguments() const {
    for (const auto &arg : registery) {
        if (arg->required && !has(arg->name)) {
            throw std::invalid_argument("Required argument not provided: " +
                                        arg->name);
        }
    }
}

inline std::vector<std::string> ArgParser::normalize_args(int argc,
                                                          char **argv) {
    std::vector<std::string> args;
    if (_program_name.empty() && argc > 0) {
        _program_name = argv[0];
    }
    args.reserve(argc > 1 ? argc - 1 : 0);

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return args;
}

#ifdef _WIN32
inline std::string ArgParser::to_utf8(const wchar_t *value) {
    // Query the required buffer length, including the terminating null byte.
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value,
                                           -1, nullptr, 0, nullptr, nullptr);
    if (length == 0) {
        throw std::runtime_error("Cannot encode image path as UTF-8");
    }

    // Encode into owned storage and remove the API's null terminator.
    std::string utf8(static_cast<std::size_t>(length), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1,
                            utf8.data(), length, nullptr, nullptr) == 0) {
        throw std::runtime_error("Cannot encode image path as UTF-8");
    }
    utf8.pop_back();
    return utf8;
}

inline std::vector<std::string> ArgParser::normalize_args(int argc,
                                                          wchar_t **argv) {
    std::vector<std::string> args;

    if (_program_name.empty() && argc > 0) {
        _program_name = argv[0];
    }
    args.reserve(argc > 1 ? argc - 1 : 0);

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(to_utf8(argv[i]));
    }

    return args;
}
#endif

inline Argument &
ArgParser::add_argument(const std::string &name,
                        const std::optional<std::string> &alias, ArgType type) {
    // Validate the argument name and alias.
    if (name.empty()) {
        throw std::invalid_argument("Argument name must not be empty");
    }

    // Throw error if argument name or alias begin with '-'
    if (name[0] == '-' || (alias.has_value() && alias.value()[0] == '-')) {
        throw std::invalid_argument(
            "Argument name should not begin with leading dashes: " + name +
            (alias.has_value() ? ", " + alias.value() : ""));
    }

    // Validate that the argument name and alias are not already registered.
    if (is_registered(name)) {
        throw std::invalid_argument("Argument already registered: " + name);
    }
    if (alias.has_value() && is_registered(alias.value())) {
        throw std::invalid_argument("Argument already registered: " +
                                    alias.value());
    }

    auto arg = std::make_shared<Argument>();
    arg->name = name;
    arg->alias = alias;
    arg->type = type;
    // for [ArgType::Flag] set the default value as false
    if (type == ArgType::Flag) {
        arg->value = "false";
    }

    registery.push_back(arg);

    // Add the argument to the name-to-argument map for both the canonical name
    // and the alias (if provided).
    name_to_arg[name] = arg;
    if (alias.has_value()) {
        name_to_arg[alias.value()] = arg;
    }

    return *arg;
}

inline bool ArgParser::has(const std::string &name) const {
    if (!is_registered(name)) {
        return false;
    }

    std::string canonical = name_to_arg.at(name)->name;
    return parsed.count(canonical) > 0;
}

inline std::string
ArgParser::help(const std::optional<std::string> &usage = std::nullopt) const {

    std::ostringstream help_str;
    if (usage != std::nullopt) {
        help_str << usage.value() << '\n';
    } else if (!_usage.empty()) {
        help_str << _usage << '\n';
    } else {
        help_str << "Usage: \n" << _program_name << '\n';
    }
    help_str << "Options: \n";

    for (const auto &arg : registery) {
        std::string option_str = "  --" + arg->name;
        if (arg->alias.has_value()) {
            option_str += ", -" + arg->alias.value();
        }
        if (arg->type == ArgType::Option) {
            option_str += " <value>";
        }
        help_str << option_str << "\n      " << arg->description << "\n";
    }

    return help_str.str();
}

inline void ArgParser::parse(int argc, char **argv) {
    auto args = normalize_args(argc, argv);
    parse(args);
}

#ifdef _WIN32
inline void ArgParser::parse(int argc, wchar_t **argv) {
    auto args = normalize_args(argc, argv);
    parse(args);
}
#endif

inline std::string arg_parser::ArgParser::at(size_t idx) const {
    size_t p_size = positional_args.size();
    if (idx >= p_size) {

        std::ostringstream error_msg;
        if (p_size > 0) {
            error_msg << "max_index is " << p_size - 1 << ", got: " << idx
                      << '\n';
        } else {

            error_msg << "no argument provided as positional\n";
        }
        throw std::out_of_range(error_msg.str());
    }

    return positional_args.at(idx);
}

inline void ArgParser::parse(const std::vector<std::string> &args) {
    bool options_ended = false;
    std::string arg;

    for (std::size_t i = 0; i < args.size(); ++i) {
        arg = args[i];

        // Handle the special case of "--" which indicates the end of options.
        if (!options_ended && arg == "--") {
            options_ended = true;
            continue;
        }

        // If options have ended or the argument does not start with a dash,
        // treat it as a positional argument.
        if (options_ended || arg.empty() || arg[0] != '-') {
            // this should be a positional argument, push it directly
            positional_args.push_back(arg);
            continue;
        }

        // Determine the start index for the argument name, skipping leading
        // dashes.
        int start_index = 1;
        if (arg.size() > 1 && arg[1] == '-') {
            start_index = 2;
        }
        // Check if the argument contains an equals sign, which indicates a
        // key-value pair.
        bool equals_sign = arg.find('=') != std::string::npos;

        // Extract the argument name and check if it is registered.
        std::string name;
        name =
            arg.substr(start_index, (equals_sign ? arg.find('=') : arg.size()) -
                                        start_index);
        if (!is_registered(name)) {
            throw std::invalid_argument("Unrecognized argument: " + arg);
        }
        // Check if the argument name or alias has already been provided.
        if (has(name)) {
            throw std::invalid_argument("Argument already provided: " + arg);
        }

        std::string value;
        auto argument = name_to_arg.at(name);

        // Extract the argument value.
        if (argument->type == ArgType::Flag) {
            if (equals_sign) {
                throw std::invalid_argument(
                    "Flag argument cannot have a value: " + arg);
            }

            value = "true";
        } else if (argument->type == ArgType::Option) {
            if (equals_sign) {
                value = arg.substr(arg.find('=') + 1);
                if (value.empty()) {
                    throw std::invalid_argument("Missing value for argument: " +
                                                arg);
                }
            } else {
                if (i + 1 >= args.size() || args[i + 1].empty() ||
                    args[i + 1][0] == '-') {
                    throw std::invalid_argument("Missing value for argument: " +
                                                arg);
                }
                value = args[++i];
            }
        }
        parsed[argument->name] = value;
    }

    // Check for required arguments after parsing all inputs.
    check_required_arguments();
};

} // namespace arg_parser

#endif // ARG_PARSER_HPP
