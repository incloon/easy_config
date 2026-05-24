# easy_config

[![Ubuntu](https://github.com/incloon/easy_config/actions/workflows/ubuntu-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/ubuntu-unit.yml)
[![Windows](https://github.com/incloon/easy_config/actions/workflows/windows-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/windows-unit.yml)
[![macOS](https://github.com/incloon/easy_config/actions/workflows/macos-unit.yml/badge.svg?branch=main)](https://github.com/incloon/easy_config/actions/workflows/macos-unit.yml)

中文 | [English](doc/README_English.md)

`easy_config` 是一个 **C++26 静态反射** 库，支持在运行时将聚合初始化语法的内容解析到自定义结构体中。利用 **C++26 P2996 静态反射**（`^^T`、`nonstatic_data_members_of`、`identifier_of`、`[:member:]`、`template for`）完全消除了外部代码生成——无需单独的编译器可执行文件、无需生成的 `.cpp` 文件、无需 CMake 代码生成管线。

> **兼容性说明**：P2996 反射（`^^T` 语法、`<meta>` 头文件）需要启用 `-freflection` 的 C++26 编译器。截至 2026-05，GCC 16.1+ 配合 `-freflection` 提供了反射 API（`^^T`、`<meta>`、`template for`）。`std::vector<info>` 所需的完整非临时 `constexpr` 分配有待 libstdc++ 更新。

### 回退模式

当没有支持反射的编译器可用时，`easy_config` 会回退到为每个用户结构体**显式模板特化** `parserDispatcher<T>`。库会提供清晰的 `static_assert` 诊断信息，引导用户编写必要的特化。

---

## 简单示例

对于下面用户定义的结构体文件 `str.h`

```c++
#pragma once
#include <string>
#include <vector>
struct Str
{
    char c;
    int i;
    float f;
    std::string str;
    ::std::vector<int> v;
};
```

`easy_config` 可以将以下结构体的聚合初始化读入结构体

```c++
//Str str =
{
    .c='\t',
    .i = (3u + 3) / 2,
    .f = 3333.e-3,
    .str = "this" " is string",
    .v = {1, 2, 3,},
};//结尾的分号可以省略
```

假设上面的聚合初始化保存在文件 `ini.txt` 中，只需要以下代码就可完成反射

```c++
#include <str.h>
#include <interpreter.hpp>

int main()
{
    ezcfg::Interpreter itp("ini.txt");
    Str str;
    itp.parse(str);
}
```

就这么简单——**无需代码生成步骤**。

---

## 快速配置

本库是纯头文件库。只需添加 include 目录：

```cmake
add_subdirectory(third/easy_config)
target_link_libraries(your_target PRIVATE ezcfg::ezcfg)
```

**无需设置 `EZCFG_STRUCT_HEADER_FILE` 变量。** 反射在编译时由库自动完成。

---

## API

```c++
class Interpreter
{
public:
    Interpreter();
    Interpreter(const std::string& str, bool is_file = true); // (1)

    bool loadFile(const std::string& file);                   // (2)
    bool loadSource(const std::string& source);               // (3)

    template<typename T, typename... TS>
    void parse(T& data, TS&... datas);                        // (4)

    ArithmeticT parseExpression();                            // (5)

    explicit operator bool() const;
};
```

| 方法 | 描述 |
|------|------|
| (1) | 构造时加载文件名或内联源字符串 |
| (2)(3) | 运行时切换解析源 |
| (4) | 批量解析结构化数据到结构体/容器 |
| (5) | 解析并求值算术表达式 |

---

## 语法支持

- C++20 指定初始化器语法：`.member = value`
- 所有标准布尔、整数和浮点数字面量
- 字符和字符串字面量（不含 Unicode 相关）
- 注释（`//`、`/* */`）和行续接（`\`）
- 算术运算符：`+`、`-`、`*`、`/`、`%`、`,`、`()`
- 所有标准容器：`vector`、`deque`、`list`、`forward_list`、`set`、`map`、`unordered_set`、`unordered_map`、`array`、`pair`
- 嵌套结构体和容器

### 特殊规则

1. 成员名称**必须**指定（强制指定初始化器）
2. 成员**不能**跳过，除非有默认值
3. 数组必须完整填充；多维数组需要显式花括号
4. `char`/`signed char`/`unsigned char` 数组可使用字符串字面量语法
5. `std::array` 视为定长数组

---

## 实现原理

`easy_config` 使用 **C++26 静态反射**（P2996）在编译时遍历结构体成员：

```c++
template <class T>
    requires (!std::is_arithmetic_v<T>)
void parserDispatcher(T& data) {
    constexpr auto members = nonstatic_data_members_of(^T);
    // template for (constexpr auto member : members) { ... }
}
```

这取代了之前独立的 `compiler` 可执行文件和 CMake 代码生成管线的架构。

---

## 错误处理

`easy_config` 使用结构化异常处理解析错误：

```c++
class ParseError : public std::runtime_error {
    // what() 返回 "文件:行:列: 错误: 消息"
    const std::string& file() const noexcept;
    size_t line() const noexcept;
    size_t column() const noexcept;
};
```

错误信息包含文件名、行号和列号，方便精确定位问题。

---

## 许可证

[GNU Lesser General Public License v3.0](COPYING.LESSER)
