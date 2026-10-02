#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace arg_parser {

enum class ArgType {
    Flag,
    Option,
    Positional
};

struct Argument {
    ArgType                    type;
    std::string                name;
    std::optional<std::string> alias;
    std::optional<std::string> default_value;
    std::string                description;
    bool                       required = false;

    Argument& set_default(const std::string& default_val);
    Argument& set_description(const std::string& desc);
    Argument& set_required(bool req);
};

class ArgParser {
private:
    std::vector<std::shared_ptr<Argument>>                     registery;
    std::vector<std::shared_ptr<Argument>>                     positional_registery;
    std::unordered_map<std::string, std::shared_ptr<Argument>> name_to_arg;
    std::unordered_map<std::string, std::string>               parsed;

    std::string program_name = "";
    std::string note         = "";

    bool is_registered(const std::string& name) const;
    void check_required_arguments() const;

    static std::vector<std::string> normalize_args(int argc, char** argv);
#ifdef _WIN32
    /**
     * @brief Encode one Windows command-line argument as UTF-8.
     *
     * @param value Non-null, null-terminated UTF-16 argument.
     *
     * @return UTF-8 string without its terminating null byte.
     */
    static std::string              to_utf8(const wchar_t* value);
    static std::vector<std::string> normalize_args(int argc, wchar_t** argv);
#endif

public:
    /**
     * @brief Set the program name for usage messages.
     */
    void set_program_name(const std::string& name)
    { program_name = name; }

    /**
     * @brief Set a note to be displayed after the help message.
     */
    void set_note(const std::string& n)
    { note = n; }

    /**
     * @brief Add a new argument to the parser, suggest chain calls to set its properties.
     *
     * @param name The name without leading dashes; must be unique.
     * @param alias The optional alias without leading dashes; must be unique.
     * @param type The type of the argument.
     *
     * @return A reference to the newly added argument.
     */
    Argument& add_argument(
        const std::string&                name,
        const std::optional<std::string>& alias = std::nullopt,
        ArgType                           type  = ArgType::Positional);

    /**
     * @brief Check if an argument has been provided.
     * Both the argument name and its alias are supported.
     * @param name The name or alias of the argument to check.
     */
    bool has(const std::string& name) const;

    /**
     * @brief Display help message for all registered arguments.
     */
    void help() const;

    /**
     * @brief
     * Get the value of an argument.
     * If the argument is registered but not provided,
     * return the default value if set, otherwise throw an exception.
     * @param name alias name are not supported.
     */
    template <typename T>
    T get(const std::string& name) const;

    /**
     * @brief parse command-line arguments.
     */
    void parse(int argc, char** argv);

#ifdef _WIN32
    /**
     * @brief parse command-line arguments (Windows version).
     */
    void parse(int argc, wchar_t** argv);
#endif

    /**
     * @brief parse normalized command-line arguments.
     */
    void parse(const std::vector<std::string>& args);
};

} // namespace arg_parser

// Implementation of template methods.

template <typename T>
T arg_parser::ArgParser::get(const std::string& name) const
{
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }

    std::string arg;
    // Handle the case where the argument is registered but not provided.
    if (!has(name)) {
        if (name_to_arg.at(name)->default_value.has_value()) {
            arg = name_to_arg.at(name)->default_value.value();
        }
        else {
            throw std::invalid_argument("Argument not provided: " + name);
        }
    }
    // Handle the case where the argument is registered and provided.
    else {
        arg = parsed.at(name);
    }

    T                 result;
    std::stringstream ss(arg);

    if (!(ss >> result)) {
        throw std::invalid_argument(
            "Failed to convert argument '" + name + "' with value '" + arg + "'");
    }
    // Check for any remaining characters in the stream after reading the value
    // For example, want a integer, input is "123abc" -> 123 is read, but "abc" remains
    ss >> std::ws;
    if (!ss.eof()) {
        throw std::invalid_argument(
            "Invalid value for argument '" + name + "': '" + arg + "'");
    }
    return result;
}

template <>
inline std::string arg_parser::ArgParser::get<std::string>(const std::string& name) const
{
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }

    // Handle the case where the argument is registered but not provided.
    if (!has(name)) {
        if (name_to_arg.at(name)->default_value.has_value()) {
            return name_to_arg.at(name)->default_value.value();
        }
        else {
            throw std::invalid_argument("Argument not provided: " + name);
        }
    }

    return parsed.at(name);
}

template <>
inline bool arg_parser::ArgParser::get<bool>(const std::string& name) const
{
    // Handle the case where the argument is not registered.
    if (!is_registered(name)) {
        throw std::invalid_argument("Argument not registered: " + name);
    }

    // Handle the case where the argument is registered but not provided.
    if (!has(name)) {
        if (name_to_arg.at(name)->default_value.has_value()) {
            return name_to_arg.at(name)->default_value.value() == "true";
        }
        else {
            throw std::invalid_argument("Argument not provided: " + name);
        }
    }

    return parsed.at(name) == "true";
}

#ifdef _WIN32
#include <windows.h>
#endif
namespace arg_parser {

/// Argument class member functions

inline Argument& Argument::set_default(const std::string& default_val)
{
    default_value = default_val;
    return *this;
}

inline Argument& Argument::set_description(const std::string& desc)
{
    description = desc;
    return *this;
}

inline Argument& Argument::set_required(bool req)
{
    required = req;
    return *this;
}

// ArgParser member functions

inline bool ArgParser::is_registered(const std::string& name) const
{
    return name_to_arg.count(name) > 0;
}

inline void ArgParser::check_required_arguments() const
{
    for (const auto& arg : registery) {
        if (arg->required && !has(arg->name)) {
            throw std::invalid_argument("Required argument not provided: " + arg->name);
        }
    }
}

inline std::vector<std::string> ArgParser::normalize_args(int argc, char** argv)
{
    std::vector<std::string> args;
    args.reserve(argc > 1 ? argc - 1 : 0);

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return args;
}

#ifdef _WIN32
inline std::string ArgParser::to_utf8(const wchar_t* value)
{
    // Query the required buffer length, including the terminating null byte.
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                           value, -1, nullptr, 0, nullptr, nullptr);
    if (length == 0) {
        throw std::runtime_error("Cannot encode image path as UTF-8");
    }

    // Encode into owned storage and remove the API's null terminator.
    std::string utf8(static_cast<std::size_t>(length), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1,
                            utf8.data(), length, nullptr, nullptr)
        == 0) {
        throw std::runtime_error("Cannot encode image path as UTF-8");
    }
    utf8.pop_back();
    return utf8;
}

inline std::vector<std::string> ArgParser::normalize_args(int argc, wchar_t** argv)
{
    std::vector<std::string> args;
    args.reserve(argc > 1 ? argc - 1 : 0);

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(to_utf8(argv[i]));
    }

    return args;
}
#endif

inline Argument& ArgParser::add_argument(
    const std::string&                name,
    const std::optional<std::string>& alias,
    ArgType                           type)
{
    // Validate the argument name and alias.
    if (name.empty()) {
        throw std::invalid_argument("Argument name must not be empty");
    }

    // FIX: report error if this argument begin with '-'
    if (name.front() == '-') {
        throw std::invalid_argument("argument name should not begin with -, use name instead of -name or --name");
    }

    // Validate that the argument name and alias are not already registered.
    if (is_registered(name)) {
        throw std::invalid_argument("Argument already registered: " + name);
    }
    if (alias.has_value() && is_registered(alias.value())) {
        throw std::invalid_argument("Argument already registered: " + alias.value());
    }

    auto arg   = std::make_shared<Argument>();
    arg->name  = name;
    arg->alias = alias;
    arg->type  = type;
    registery.push_back(arg);

    // Add the argument to the name-to-argument map for both the canonical name and the alias (if provided).
    name_to_arg[name] = arg;
    if (alias.has_value()) {
        name_to_arg[alias.value()] = arg;
    }

    // If the argument is positional, add it to the positional registry.
    if (type == ArgType::Positional) {
        positional_registery.push_back(arg);
    }

    return *arg;
}

inline bool ArgParser::has(const std::string& name) const
{
    if (!is_registered(name)) {
        return false;
    }

    std::string canonical_name = name_to_arg.at(name)->name;
    return parsed.count(canonical_name) > 0;
}

inline void ArgParser::help() const
{
    std::string usage = "Usage: " + program_name + " [options]";
    for (const auto& arg : positional_registery) {
        usage += " <" + arg->name + "> ";
    }
    std::cout << usage << "\n\n";
    std::cout << "Options:\n";
    for (const auto& arg : registery) {
        std::string option_str = "  --" + arg->name;
        if (arg->alias.has_value()) {
            option_str += ", -" + arg->alias.value();
        }
        if (arg->type == ArgType::Option) {
            option_str += " <value>";
        }
        std::cout << option_str << "\n      " << arg->description << "\n";
    }
    if (!note.empty()) {
        std::cout << "\n"
                  << note << "\n";
    }
}

inline void ArgParser::parse(int argc, char** argv)
{
    auto args = normalize_args(argc, argv);
    parse(args);
}

#ifdef _WIN32
inline void ArgParser::parse(int argc, wchar_t** argv)
{
    auto args = normalize_args(argc, argv);
    parse(args);
}
#endif

inline void ArgParser::parse(const std::vector<std::string>& args)
{
    bool        options_ended    = false;
    std::size_t positional_index = 0;
    std::string arg;

    for (std::size_t i = 0; i < args.size(); ++i) {
        arg = args[i];

        // Handle the special case of "--" which indicates the end of options.
        if (!options_ended && arg == "--") {
            options_ended = true;
            continue;
        }

        // If options have ended or the argument does not start with a dash, treat it as a positional argument.
        if (options_ended || arg.empty() || arg[0] != '-') {
            if (positional_index >= positional_registery.size()) {
                throw std::invalid_argument("Unexpected positional argument: " + arg);
            }

            auto positional_arg          = positional_registery[positional_index++];
            parsed[positional_arg->name] = arg;
            continue;
        }

        // Determine the start index for the argument name, skipping leading dashes.
        int start_index = 1;
        if (arg.size() > 1 && arg[1] == '-') {
            start_index = 2;
        }
        // Check if the argument contains an equals sign, which indicates a key-value pair.
        bool equals_sign = arg.find('=') != std::string::npos;

        // Extract the argument name and check if it is registered.
        std::string name;
        name = arg.substr(
            start_index,
            (equals_sign ? arg.find('=') : arg.size()) - start_index);
        if (!is_registered(name)) {
            throw std::invalid_argument("Unrecognized argument: " + arg);
        }
        // Check if the argument name or alias has already been provided.
        if (has(name)) {
            throw std::invalid_argument("Argument already provided: " + arg);
        }

        std::string value;
        auto        argument = name_to_arg.at(name);
        if (argument->type == ArgType::Positional) {
            throw std::invalid_argument("Positional argument cannot be provided as an option: " + arg);
        }
        // Extract the argument value.
        if (argument->type == ArgType::Flag) {
            if (equals_sign) {
                throw std::invalid_argument("Flag argument cannot have a value: " + arg);
            }

            value = "true";
        }
        else if (argument->type == ArgType::Option) {
            if (equals_sign) {
                value = arg.substr(arg.find('=') + 1);
                if (value.empty()) {
                    throw std::invalid_argument("Missing value for argument: " + arg);
                }
            }
            else {
                if (i + 1 >= args.size() || args[i + 1].empty() || args[i + 1][0] == '-') {
                    throw std::invalid_argument("Missing value for argument: " + arg);
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
