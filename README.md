# arg_parser

A lightweight C++17 single-header command-line parser with no third-party dependencies.

## Supported forms

| Type | Examples |
| --- | --- |
| Flag | `--verbose`, `-v` |
| Option | `--output=result.txt`, `--output result.txt`, `-o=result.txt`, `-o result.txt` |
| Positional | `photo.png` |
| End of options | `-- -photo.png` (keeps the leading dash) |

## Installation

As this lib is header-only, just install the `hpp` file then include it.

- Install

```bash
wget https://github.com/IridiumNan/cpp-arg-parser/raw/refs/heads/main/arg_parser.hpp

# You can also install by github web UI
```

- Include

```cpp
#include "arg_parser.hpp"
```

## Quick start

Just use this lib with 3 steps

This can be used as follows

- Step 1: Add expected arguments

- Step 2: Parse all args

- Step 3: Fetch argument with type assign or fetch by index (for positional args)

```cpp
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
```

```bash
g++ -std=c++17 example.cpp -o example
./example photo.png -o result.txt -v

# output
1 pos: photo.png
verbose: 1
output: result.txt
```

On Windows with MinGW, use `wmain` and add `-municode`:

```cpp
int wmain(int argc, wchar_t* argv[])
```

## API

| Function | Description |
| --- | --- |
| `add_argument(name, alias = nullopt, type = ArgType::Flag)` | Register an argument. |
| `set_default(value)` | Set a default value. |
| `set_description(text)` | Set the help description. |
| `set_required(true)` | Throw if the argument is not provided. |
| `parse(argc, argv)` | Parse `main` arguments (skips `argv[0]`). |
| `has(name)` | Check if provided, by name or alias. |
| `get<T>(name)` | Get a typed value, by name or alias. |
| `help(usage)` | Return the help text. |
| `size()` / `at(i)` | Access positional arguments. |

## Notes

- Values starting with `-` require `=`: `--count=-12` works, `--count -12` does not.
- Combined short flags like `-vh` are not supported.
- Call `parse()` once per instance.
- `-h` / `--help` are not automatic — check `parser.has("help")` yourself.

## License

[MIT](/LICENSE)
