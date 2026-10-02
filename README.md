# arg_parser

A lightweight C++17 single-header command-line parser with no third-party dependencies.

## Usage

| Type | Supported forms |
| --- | --- |
| Flag | `--flag`, `-f` |
| Option | `--option=value`, `--option value`, `-o=value`, `-o value` |
| Positional | `photo.png` |
| End of options | `-- -photo.png` (preserves the leading dash) |

```sh
./example photo.png -o result.txt -c 120 -v
./example "my photo.png" --output=result.txt --count 120
./example --count=-12
./example -h
./example --help
```

Flag takes no value, option takes a single value, and positional takes a value without a flag.
Arguments are optional unless marked with `set_required(true)`.
The end of options marker `--` allows positional arguments that start with a dash.
For option values starting with a dash, use `=`, such as `--count=-12` or `--output=-result.txt`;
the space-separated form `--count -12` is rejected.
Combined short flags such as `-vh` are **not** supported.

## Features

- **Header-only:** A single C++17 header with no third-party dependencies.
- **Argument parsing:** Supports flags, options, positional arguments, and short aliases.
- **Argument configuration:** Supports default values, required arguments, and typed retrieval with `get<T>()`.
- **Built-in help:** Generates usage information from registered arguments.
- **Windows Unicode support:** Converts UTF-16 arguments from `wmain` to UTF-8.

## Install

Clone the repository:

```sh
git clone https://github.com/KAI-SHUNG/arg_parser.git arg_parser
cd arg_parser
```

Copy `arg_parser.hpp` into your project's include directory, then use:

```cpp
#include "arg_parser.hpp"
```

## Quick start

This can be used as follows

- Step 1: **Add expected arguments**

- Step 2: **Parse all args**

- Step 3: **Fetch argument with type assign**

Here is an comprehensive example

```cpp
#include "arg_parser.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    using arg_parser::ArgType;
    arg_parser::ArgParser parser;

    // step 1: add expected args
    /// description, default value can be setted by set_xxx function
    parser.set_program_name("example");
    parser.add_argument("input").set_default("input.png").set_description("Input file");
    parser.add_argument("output", "o", ArgType::Option).set_default("output.txt");
    parser.add_argument("count", "c", ArgType::Option).set_default("80");
    parser.add_argument("verbose", "v", ArgType::Flag).set_default("false");
    parser.add_argument("help", "h", ArgType::Flag).set_description("Show help");

    // step 2: parse all args
    // using try-catch is recommended
    try {
        parser.parse(argc, argv);
    }
    catch (const std::exception& error) {
        std::cerr << "Failed to parse arguments: " << error.what() << '\n';
        parser.help();
        return 22;
    }
    
    // if help is provided, parser can print a comprehensive help manual for all arguments registered
    if (parser.has("help")) { parser.help(); return 0; }

    // step 3: fetch argument with type assign
    // it will return default value if it is not provided and has default value
    std::cout << "output: " << parser.get<std::string>("output") << '\n';
    std::cout << parser.get<int>("count") << '\n'
    
    return 0;
}
```

> [!NOTE]
> It will throw error if you add an argument starting with a leading dash
>
> For example, `parser.add_argument("--test", "-t", ArgType::Flag);`

Value retrieval and exception handling are omitted here for brevity;
see [example.cpp](example.cpp) for the complete example.

On Windows, the example uses `wmain` to preserve Unicode arguments.

Compile `example.cpp`:

```sh
g++ -std=c++17 example.cpp -o example
./example photo.png -o result.txt -c 120 -v
```

On Windows with MinGW, add `-municode`:

```sh
g++ -std=c++17 -municode example.cpp -o example.exe
./example.exe photo.png -o result.txt -c 120 -v
```

Run `./example -h` (or `./example --help`) to show help, including the input description:

The application must call `help()` explicitly; registering a help flag does not display help automatically.

```text
Usage: example [options] <input>

Options:
  --input
      Input file
  --output, -o <value>

  --count, -c <value>

  --verbose, -v

  --help, -h
      Show help
```

Registration: `add_argument(name, alias = std::nullopt, type = ArgType::Positional)`.

| Parameter / setting | Description |
| --- | --- |
| `name` | Unique canonical name, without dashes |
| `alias` | Optional alias, without dashes |
| `type` | `Positional`, `Option`, or `Flag` |
| `set_default(value)` | Default value as a string |
| `set_description(text)` | Description shown in help |
| `set_required(true)` | Require explicit input; a default does not satisfy it |

Public functions (`ArgParser`):

| Function / parameters | Return type | Description |
| --- | --- | --- |
| `add_argument(name, alias = std::nullopt, type = ArgType::Positional)` | `Argument&` | Register an argument; supports chained configuration. |
| `set_program_name(name)` | `void` | Set the program name shown in help. |
| `set_note(text)` | `void` | Add a note after the help message. |
| `parse(int argc, char** argv)` | `void` | Parse `main` arguments, skipping `argv[0]`. |
| `parse(int argc, wchar_t** argv)` | `void` | Windows: parse `wmain` arguments as UTF-8, skipping `argv[0]`. |
| `parse(const std::vector<std::string>& args)` | `void` | Parse an argument list without the executable name. |
| `has(name)` | `bool` | Check explicit input by name or alias; defaults do not count. |
| `get<T>(name)` | `T` | Get a typed value or default by canonical name; throws on missing or invalid values. |
| `help()` | `void` | Print usage and argument descriptions. |

Use canonical names with `get<T>()`. Call `parse()` once per instance. Windows additionally supports `parse(argc, wchar_t**)` for UTF-16 input through `wmain`.

## Tests

Compile and run the test suite from the repository root:

```sh
g++ -std=c++17 -I. tests/test_arg_parser.cpp -o test_arg_parser
./test_arg_parser
```

On Windows with MinGW:

```sh
g++ -std=c++17 -I. tests/test_arg_parser.cpp -o test_arg_parser.exe
./test_arg_parser.exe
```

The test runner uses `main`, so `-municode` is not needed. A nonzero exit code indicates a failure.

## License

Licensed under the [MIT License](LICENSE).
