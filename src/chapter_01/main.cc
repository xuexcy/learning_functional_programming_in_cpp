/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/06/04 15:37:39
# Desc   : chapter_01: 函数式编程简介
########################################################################
*/

#include <algorithm>
#include <cmath>
#include <execution>
#include <fstream>
#include <iterator>
#include <ranges>
#include <vector>

#include "cpp_utils/util.h"

// 1.1 什么是函数式编程
/*
1. 纯函数 Pure Function
2. 惰性求值 Lazy Evaluation
3. 模式匹配 Pattern Matching

求和
命令式编程: 提供步骤，指明求和的每一个步骤
函数式编程: 提供定义，把第一个元素和剩余元素相加
*/
// 1.1.1 与面向对象编程的关系
/*
面向对象OOP: 基于对数据的抽象，隐藏对象的内部细节，暴露对象的 API
FP: 基于对函数的抽象, 创建比地城语言提供的复杂得多的控制结构
*/
// 1.1.2 命令式与声明式编程的比较
// 统计文件行数
// 命令式
std::vector<int> count_lines_in_files(const std::vector<std::string>& files) {
  std::vector<int> results;
  char c = 0;
  for (const auto& file : files) {
    int line_count = 0;
    std::ifstream in(file);
    while (in.get(c)) {
      if (c == '\n') {
        ++line_count;
      }
    }
    results.push_back(line_count);
  }
  return results;
}
// 使用 std::count
// 这里不再需要关心如何进行统计(比如计数、循环, 越多的步骤，越多的细节就越可能导致程序写错)，只需要知道统计流中的换行符数量
// 函数式编程的主要思想: 使用抽象来表述用户的目的，而不是如何去做
// 用户要啥就是啥，至于怎么做不重要
int count_lines(const std::string& filename) {
  std::ifstream in(filename);
  return std::count(
      std::istreambuf_iterator<char>(in),
      std::istreambuf_iterator<char>(),
    '\n');
}
std::vector<int> count_lines_in_files_2(const std::vector<std::string>& files) {
  std::vector<int> results;
  // 使用 std::transform 将这一步也函数式风格化
  for (const auto& file : files) {
    results.push_back(count_lines(file));
  }
  return results;
}
// 使用 std::transform
std::vector<int> count_lines_in_files_3(const std::vector<std::string>& files) {
  std::vector<int> results(files.size());
  std::transform(files.begin(), files.end(), results.begin(), count_lines);
  return results;
}
// 使用管道操作符 + std::ranges::transform
std::vector<int> count_lines_in_files_4(const std::vector<std::string>& files) {
  return files | std::views::transform(count_lines) | std::ranges::to<std::vector>();
}
// 上面count_lines接受一个string，但不清楚是文件名还是字符串
std::ifstream open_file(const std::string& filename) {
  return std::ifstream(filename);
}
int count_lines(std::ifstream& in) {
  return std::count(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), '\n');
}
std::vector<int> count_lines_in_files_5(const std::vector<std::string>& files) {
  return files | std::views::transform(open_file) | std::views::transform([](std::ifstream in) {
      return count_lines(in);
  }) | std::ranges::to<std::vector>();
  /*
  The error occurs because std::views::transform expects the transformation function to be copyable, but std::ifstream is non-copyable, so you need to use std::move to transfer ownership of the ifstream to count_lines.
  不能写成 std::views::transform(count_lines), 因为 count_lines 不是 copyable 的(它的参数 std::ifstream 不是 copyable的)
  所以需要 lambda 通过传值的方式将 std::ifstream 保留下来
  */
  // return files | std::views::transform(open_file) | std::views::transform(count_lines) | std::ranges::to<std::vector>();
}

// 1.2 纯函数(Pure functions)
/*
FP 的核心思想是纯函数: 函数值使用参数来计算结果，多次调用都会得到相同的结果，且不留痕迹(比如有一个全局变量在记录调用次数)
*/
// 1.2.1 避免可变状态
// 减少函数内的可变量

// 1.3 以函数方式思考问题
/*
需要什么样的转换，而不是具体算法步骤
*/

// 1.4 函数式编程的优点
// 1.4.1 代码简洁易读
// 1.4.2 并发和同步
/*
并发的难点在于共享可变的状态, 但纯函数不修改任何东西
*/
void run_sum() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<double> vec = {1.0, 2.0, 3.0};
  // sqrt 有多个重载版本，需要使用 static_cast 指明使用的版本
  auto result = vec | std::views::transform(static_cast<double(*)(double)>(std::sqrt));
  auto sum = std::reduce(std::execution::par, result.begin(), result.end(), 0.0);
  std::println("sum(vec)={}", sum);
  std::println();
}

// 1.4.3 持续优化

// 1.5 C++ 作为函数式编程语言的进化

// 1.6 将会学到什么
int main() {
  run_sum();
  return 0;
}
