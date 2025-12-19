/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/12/18 14:54:28
# Desc   : 第 13 章 测试与调试
########################################################################
*/

#include "cpp_utils/util.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <ratio>
#include <ranges>
#include <string>
#include <vector>

// 13.1 程序编译正确吗？
template <typename Representation, typename Ratio = std::ratio<1>>
class Distance {
 public:
  constexpr explicit Distance(Representation v) : value(v) {}
  Distance operator+(const Distance& other) const {
    return Distance{value + other.value};
  }
  Representation value{};
};  // class Distance

template <typename Representation>
using meters = Distance<Representation>;

template <typename Representation>
using kilometers = Distance<Representation, std::kilo>;

template <typename Representation>
using centimeters = Distance<Representation, std::centi>;

template <typename Representation>
using miles = Distance<Representation, std::ratio<1609, 1>>;

namespace distance_literals {
constexpr kilometers<long double> operator ""_km(long double distance) {
  return kilometers<long double>{distance};
}

constexpr miles<long double> operator ""_mi(long double distance) {
  return miles<long double>{distance};
}
}  // namespace distance_literals

void run_distance() {
  PRINT_CURRENT_FUNCTION_NAME;
  using namespace distance_literals;
  auto d1 = 3.0_km;
  auto d2 = 4.0_mi;
  auto _ = d1 + d1;
  // auto _ = d1 + d2;  // 编译错误

}

// 13.2 单元测试与纯函数
// 纯函数没有状态，单元测试不需要初始化状态
// 单元测试的任务是隔离小部分程序并独立进行测试。任何纯函数都已经是程序的独立部分，再加上纯函数易于测试的事实，可以
// 使每个纯函数适用于单元测试。

template <typename Iter, typename End>
int count_lines(const Iter& begin, const End& end) {
  using std::count;
  return count(begin, end, '\n');
}
template <typename Input>
int count_lines(const Input& input) {
  return count_lines(std::begin(input), std::end(input));
}

void run_count_lines() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string s = "Hello\nworld\n";
  assert(count_lines(std::begin(s), std::end(s)) == 2);

  auto r = s | std::views::transform([](char c) { return std::toupper(c); });
  assert(count_lines(std::begin(r), std::end(r)) == 2);
}

// 13.3 自动产生测试
/*
单元测试需要手工编写，有可能编写错误的单元测试
*/

// 13.3.1 产生测试用例
static constexpr int MAX_STRING_LENGTH = 1000;
static constexpr int MAX_LINE_COUNT = 100;
static constexpr int TEST_CASES_PER_LINE_COUNT = 10;

std::string generate_random_string() {
  int char_number = rand() % MAX_STRING_LENGTH;
  std::string result;

  for (int i = 0; i < char_number; ++i) {
      char c = rand() % 255;
      if (c == '\n') continue;
      result += c;
  }

  return result;
}

std::string generate_test_case(int line_count) {
  std::string result;
  for (int i = 0; i < line_count; ++i) {
      result += generate_random_string() + '\n';
  }
  result += generate_random_string();
  return result;
}

auto generate_test_cases(int line_count) {
  return std::views::iota(0, TEST_CASES_PER_LINE_COUNT) |
      std::views::transform([line_count](int) { return generate_test_case(line_count); });
}
void run_generated_tests() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto seed = time(nullptr);
  // 输出种子，当测试出错时可以知道哪个版本发生了错误
  // std::println("Random seed is: {}", seed);
  srand(seed);

  for (int line_count = 0; line_count <= MAX_LINE_COUNT; ++line_count) {
    for (const auto& test : generate_test_cases(line_count)) {
      assert(count_lines(test) == line_count);
    }
  }
}

// 13.3.2 基于规则的测试
template <typename T>
void test_reverse_by_rule(const T& t) {
  const auto rev_t = reverse(t);
  assert(t == reverse(rev_t));
  assert(t.size() == rev_t.size());
  assert(t.front() == rev_t.back());
  assert(t.back() == rev_t.front());
}
template <typename T>
void test_sort_by_rule(const T& t) {
  const auto sorted_t = sort(t);
  assert(t.size() == sorted_t.size());
  assert(std::min(t) == sorted_t.front());
  assert(std::is_sorted(std::begin(sorted_t), std::end(sorted_t)));
  assert(sorted_t == sort(reverse(t)));
}

void run_test_by_rule() {
  PRINT_CURRENT_FUNCTION_NAME;
}

// 13.3.3 比较测试

// 13.4 测试基于 monad 的并发系统
int main() {
  run_distance();
  run_count_lines();
  run_generated_tests();
  run_test_by_rule();
  return 0;
}

