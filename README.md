# C++函数式编程(functional Programming in C++)
- pdf: [C++函数式编程](https://github.com/xuexcy/public_attachments/blob/main/books/C%2B%2B%E5%87%BD%E6%95%B0%E5%BC%8F%E7%BC%96%E7%A8%8B%20(%5B%E5%A1%9E%E5%B0%94%E7%BB%B4%E4%BA%9A%5D%E4%BC%8A%E5%87%A1%C2%B7%E5%BA%93%E5%A5%87)%20(Z-Library).pdf)
- 豆瓣链接: https://book.douban.com/subject/34923168/
- 原书代码: https://gitlab.com/manning-fpcpp-book/code-examples

## Build And Run
- `env_variable.sh`: 设置环境变量，如 cmake 工作目录、可执行文件存放的 bin 文件夹目录等
- `build.sh`: 编译所有 src/*.cc
- `run.sh`: 运行所有编译后的可执行文件，并将结果输出到 stdout 目录
- `build_one.sh`: 编译 src 目录下的一个 .cc 文件，具体编译哪一个文件取决于 `env_variable.sh` 中的 `default_execute_file` 环境变量。 这个脚本主要是自己修改、测试某个 .cc 代码的时候使用，该文件里没有调用 cmake，如果编译的相关依赖发生改变，请重新执行 `build.sh` 生成新的 makefile 文件
- `run_one.sh`: 执行一个编译结果，同 `build_one.sh` 一样，具体哪个取决于 `env_variable.sh` 中的 `default_execute_file`
- br_one.sh: 编译一个并执行
- `src/*.cc`: 各个 main 文件

# 目录
1. 函数式编程简介
2. 函数式编程之旅
3. 函数对象
4. 以旧函数创建新函数
5. 纯洁性: 避免可变状态
6. 惰性求值
7. range
8. 函数式数据结构
13. 测试与调试

# 其他
1. 使用 g++ 编译，因为 clang 现在(2025/06/05)还没有支持 `std::execution::par`

