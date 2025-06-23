/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/22 16:45:34
# Desc   : 第 7 章 range
########################################################################
*/

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <ranges>
#include <source_location>
#include <sstream>
#include <vector>

#include "cpp_utils/util.h"
#include "range/v3/action.hpp"
#include "range/v3/range.hpp"
#include "range/v3/range_fwd.hpp"
#include "range/v3/view/istream.hpp"

// 7.1 range 简介

// 7.2 创建数据的只读视图

// 7.2.1 range 的 filter 函数
/*
filter
auto& operator++() {
  ++cur_pos_;
  cur_pos_ = std::find_if(cur_pos_, end_, pred_);
  return *this;
}
*/

// 7.2.2 range 的 transform 函数
/*
transform
auto operator*() const {
  return func_(*cur_pos_);

*/

// 7.2.3 range 惰性求值
/*
std::vector<std::string> names = people | filter(is_female) | transform(name) | take(3)
range 视图的求值是惰性的，当对集合调用 filter 和 transform 时，只是定义了一个视图，并没有对任何一个元素求值
生成视图:
  1. 当调用 filter(is_female) 时创建一个视图，将迭代器指向第一个符合谓词 is_female 的元素
  2. 当调用 transform 时创建一个视图，其他什么也不用做
  3. 当调用 take(3) 时创建一个视图
  4. 从 take(3) 转换的结果创建一个 std::vector<std::string>


*/

// 7.3 修改 range 中的值
// 修改源集合中的元素，这种操作称为行为 action, 行为与视图需要区分开来(视图不改变源集合)

template <typename It>
void print(It begin, It end) {
  for (auto it = begin; it != end; ++it) {
    std::print("{} ", *it);
  }
  std::println();
}

void run_action() {
  std::println("{}", std::source_location::current().function_name());
  {
    std::println("std::ranges");
    std::vector<std::string> vec{"a", "a", "c", "a", "b", "d", "b", "c", "g" };
    print(vec.begin(), vec.end());
    std::ranges::sort(vec);  // 修改源集合
    auto [first, last] = std::ranges::unique(vec);  // 修改源集合
    vec.erase(first, last);
    print(vec.begin(), first);
    assert(first == vec.end());
  }
  {
    std::println("range-v3");
    // C++20 的 ranges 中没有包含 actions, 这里使用 range-v3 库的 actions
    std::vector<std::string> vec{"a", "a", "c", "a", "b", "d", "b", "c", "g" };
    vec |= ranges::actions::sort;
    vec |= ranges::actions::unique;
    print(vec.begin(), vec.end());
  }
  std::println();
}

// 7.4 定界 range 和 无限 range
// 哨兵: sentinel

// 7.4.1 定界 range
// delimited range: 不能事先知晓其大小，但可以通过一个谓词检测是否已经到达末尾
template <typename T>
bool operator==(const std::istream_iterator<T>& left, const std::istream_iterator<T>& right) {
  if (left.is_sentinel() && right.is_sentinel()) {
    return true;  // 两个迭代器都是哨兵
  } else if (left.is_sentinel()) {
    // 检测 right 是否满足谓词
  } else if (right.is_sentinel()) {
    // 检测 left 是否满足谓词

  } else {
    return *left == *right;  // 两个迭代器都不是哨兵，比较它们的值
  }
}

// 7.4.2 用哨兵创建无限 range

template <typename Range>
void write_top_10(const Range& xs) {
  auto items = std::views::zip(xs, std::views::iota(1)) | std::views::transform([](const auto& tuple) {
    return std::format("{} {}", std::get<1>(tuple), std::get<0>(tuple));
  }) | std::views::take(10);
  for (const auto& item : items) {
    std::println("{}", item);
  }
}

void run_unbounded_range() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<std::string> movies{ "Meaning of life", "Dr Strangelove" };
  write_top_10(movies);
  std::println();
}

// 7.5 用 range 统计词频
void run_word_frequency() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string s{"a  a  c A a C b d b   , c g"};
  std::stringstream ss(s);
  auto words = std::ranges::istream_view<std::string>(ss)
      | std::views::transform([](std::string word) {
          std::ranges::transform(word, word.begin(), static_cast<int(*)(int)>(std::tolower));
          return word;
        })
      | std::views::filter([](const std::string& w) {
          return std::ranges::all_of(w, [](unsigned char c) { return std::isalnum(c); });
        })
      | std::views::filter([](const std::string& w) { return !w.empty(); })
      | std::ranges::to<std::vector>();
  std::ranges::sort(words);
  auto result = words | std::views::chunk_by(std::equal_to<>())
      | std::views::transform([](const auto& group) {
        auto begin = std::begin(group);
        auto count = std::distance(begin, std::end(group));
        return std::make_pair(count, *begin);
      })
      | std::ranges::to<std::vector>();
  std::ranges::sort(result);
  std::ranges::reverse(result);
  print(result.begin(), result.end());

  std::println();

}
int main() {
  run_action();
  run_unbounded_range();
  run_word_frequency();
  return 0;
}
