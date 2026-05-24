# easy_config

[![Ubuntu](https://github.com/incloon/easy_config/actions/workflows/ubuntu-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/ubuntu-unit.yml)
[![Windows](https://github.com/incloon/easy_config/actions/workflows/windows-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/windows-unit.yml)
[![macOS](https://github.com/incloon/easy_config/actions/workflows/macos-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/macos-unit.yml)
[![codecov](https://codecov.io/gh/incloon/easy_config/branch/main/graph/badge.svg)](https://codecov.io/gh/incloon/easy_config)

English | [中文](doc/README_Chinese.md)

`easy_config` is a **C++26 static reflection** library for runtime parsing of aggregate initialization syntax into user-defined structs. Powered by **P2996 static reflection** (`^^T`, `nonstatic_data_members_of`, `identifier_of`, `[:member:]`, `template for`), it eliminates external code generation entirely — no separate compiler, no generated `.cpp` files, no CMake code-gen pipeline.

> **Note**: Requires GCC 16.1+ with `-std=c++26 -freflection`. A fallback path with explicit template specializations is available for compilers without reflection support.

---

## Quick Example

Struct in `str.h`:

```cpp
struct Str {
    char c;
    int i;
    float f;
    std::string str;
    std::vector<int> v;
};
```

Input in `ini.txt`:

```
{
    .c = '\t',
    .i = (3u + 3) / 2,
    .f = 3333.e-3,
    .str = "this" " is string",
    .v = {1, 2, 3,},
};
```

Code:

```cpp
#include <str.h>
#include <interpreter.hpp>

int main() {
    ezcfg::Interpreter itp("ini.txt");
    Str str;
    itp.parse(str);
}
```

---

## Setup

```cmake
add_subdirectory(third/easy_config)
target_link_libraries(your_target PRIVATE ezcfg::ezcfg)
```

That's it — no `EZCFG_STRUCT_HEADER_FILE`, no code-gen step.

---

## API

```cpp
class Interpreter {
public:
    Interpreter();
    Interpreter(const std::string& str, bool is_file = true);

    bool loadFile(const std::string& file);
    bool loadSource(const std::string& source);

    template<typename T, typename... TS>
    void parse(T& data, TS&... datas);

    ArithmeticT parseExpression();
    explicit operator bool() const;
};
```

---

## Error Handling

Structured exceptions with file/line/column info:

```cpp
class ParseError : public std::runtime_error {
    // what() → "file:line:col: error: message"
    const std::string& file() const noexcept;
    size_t line() const noexcept;
    size_t column() const noexcept;
};
```

---

## Supported Syntax

- C++20 designated initializers (`.member = value`)
- Boolean, integer, floating-point literals
- Character and string literals
- Comments (`//`, `/* */`) and line continuations (`\`)
- Arithmetic: `+`, `-`, `*`, `/`, `%`, `,`, `()`
- All STL containers and nested structs

---

## License

GNU Lesser General Public License v3.0
