# arg_parser

一个轻量级的 C++17 单头文件命令行解析器，无第三方依赖。

[![CI](https://github.com/IridiumNan/cpp-arg-parser/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/IridiumNan/cpp-arg-parser/actions/workflows/cmake-multi-platform.yml)

适合需要快速集成命令行解析的 C++ 项目。

## 支持的形式

| 类型 | 示例 |
| --- | --- |
| 标志 | `--verbose`, `-v` |
| 选项 | `--output=result.txt`, `--output result.txt`, `-o=result.txt`, `-o result.txt` |
| 位置参数 | `photo.png` |
| 选项结束符 | `-- -photo.png`（保留开头的短横线） |

## 安装

本库为 header-only，只需安装 `hpp` 文件并包含即可。

- 安装

**命令行**

```bash
wget https://github.com/IridiumNan/cpp-arg-parser/raw/refs/heads/main/arg_parser.hpp

# 或者使用 curl
# curl -fsSL https://github.com/IridiumNan/cpp-arg-parser/raw/refs/heads/main/arg_parser.hpp -o arg_parser.hpp
```

**浏览器**

[github 下载](https://github.com/IridiumNan/cpp-arg-parser/raw/refs/heads/main/arg_parser.hpp)

- 包含

```cpp
#include "arg_parser.hpp"
```

## CMake

如果你使用 CMake, 可以添加为子目录

```cmake
add_subdirectory(cpp-arg-parser)
target_link_libraries(your_app PRIVATE arg_parser::arg_parser)
```

## 快速开始

> [!NOTE]
> 最佳实践  
> 参见 [best_practice.cpp](./best_practice.cpp)

只需三步即可使用本库。

用法如下：

- 第 1 步：添加期望的参数

- 第 2 步：解析所有参数

- 第 3 步：通过类型赋值获取参数，或按下标获取位置参数

```cpp
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

    // 或者通过 set_xxx 函数设置程序名
    // parser.set_program_name("cpp");
    // parser.set_usage(usage);

    // 第 1 步：设置需要的参数
    // 支持 [ArgType::Option] 和 [ArgType::Flag]
    // 默认为 Flag
    // 对于 Flag，默认值为 false，无需手动设置
    parser.add_argument("verbose", "v")
        .set_description("如果为 true，打印详细输出");

    parser.add_argument("output", "o", ArgType::Option)
        .set_description("将输出放入 <file>。")
        .set_required(true);

    parser.add_argument("help", "h")
        .set_description("打印帮助手册");

    try {
        // 第 2 步：直接解析参数
        // 警告：除非你知道自己在做什么，否则不要使用 vector 版本
        parser.parse(argc, argv);

        // 第 3 步：遍历位置参数
        // 通过类型赋值获取选项值
        for (size_t i = 0; i < parser.size(); i++) {
            std::cout << i << " pos: " << parser.at(i) << '\n';
        }
        std::cout << "verbose: " << parser.get<bool>("v") << '\n';
        std::cout << "output: " << parser.get<std::string>("o") << '\n';

    } catch (const std::exception &err) {
        std::cerr << err.what() << '\n';
        // 如果已经设置过 usage，这里留空即可
        std::cout << parser.help();
        return 1;
    }
}
```

- 编译

```bash
g++ -std=c++17 example.cpp -o bin/example
```

- 运行

```bash
bin/example photo.png -o result.txt -v

# 输出
0 pos: photo.png
verbose: 1
output: result.txt
```

```bash
bin/example -o bin/main hello -- --port world

# 输出
# -- 之后的所有参数都会被视为位置参数
0 pos: hello
1 pos: --port
2 pos: world
verbose: 0
output: bin/main
```

- 自动生成帮助手册（针对选项）

```bash
bin/example example.cpp main.cpp
# 输出
Required argument not provided: output
Usage: 
g++ [options] file...

Options: 
  --verbose, -v
      if true, print with verbose output
  --output, -o <value>
      Place the output into <file>.
  --help, -h
      print help manual
```

在 Windows 上使用 MinGW 时，使用 `wmain` 并添加 `-municode`：

```cpp
int wmain(int argc, wchar_t* argv[])
```

```bash
g++ -std=c++17 -municode example.cpp -o example.exe
```

## API

### ArgParser

| 函数 | 说明 |
| --- | --- |
| `ArgParser(program_name, usage)` | 创建一个新的解析器 |
| `add_argument(name, alias = nullopt, type = ArgType::Flag)` | 注册一个参数。 |
| `set_program_name(program_name)` | 设置解析器的程序名 |
| `set_usage(usage)` | 设置解析器的用法 |
| `parse(argc, argv)` | 解析 `main` 参数（跳过 `argv[0]`）。 |
| `has(name)` | 检查是否提供，支持名称或别名。 |
| `get<T>(name)` | 获取类型化值，支持名称或别名。 |
| `help()` | 返回帮助文本。 |
| `size()` / `at(i)` | 访问位置参数。 |

### Argument

| 函数 | 说明 |
| --- | --- |
| `set_default(value)` | 设置默认值。 |
| `set_description(text)` | 设置帮助描述。 |
| `set_required(true)` | 如果未提供该参数则抛出异常。 |

## 注意事项

- 以 `-` 开头的值需要使用 `=`：`--count=-12` 可以，`--count -12` 不行。
- 不支持组合短标志，如 `-vh`。
- 每个实例只能调用一次 `parse()`。
- `-h` / `--help` 不会自动处理 —— 请自行检查 `parser.get<bool>("help")`。

## Build and test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To build examples as well:

```bash
cmake -S . -B build -DARG_PARSER_BUILD_EXAMPLES=ON
cmake --build build
```

## 致谢

感谢 [KAI-SHUNG](https://github.com/KAI-SHUNG) 编写了本解析器的第一个版本

参见 <https://github.com/KAI-SHUNG/arg_parser>

## 许可证

[MIT](./LICENSE)

---

欢迎试用和反馈，如果觉得有用可以点个 Star。
