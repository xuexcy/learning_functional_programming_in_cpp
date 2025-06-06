/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/05 13:06:07
# Desc   :
########################################################################
*/

#include <algorithm>
#include <cassert>
#include <execution>
#include <functional>
#include <numeric>
#include <print>
#include <ranges>
#include <vector>
#include "cpp_utils/util.h"

// 2.1 函数使用函数?
/*
输出集合中女性的名字
is_female
  1. filter: (collection<T>, (T -> bool)) -> collection<T>
  2. transform: (collection<T>, (T -> Out)) -> collection<Out>
*/

// 2.2 STL 实例
// 2.2.1 求平均值
double average_score(const std::vector<int>& scores) {
  int sum = 0;
  for (int score : scores) {
    sum += score;
  }
  return sum / (double)(scores.size());
}
// 使用 std::accumulate
double average_score_2(const std::vector<int>& scores) {
  return std::accumulate(scores.begin(), scores.end(), 0) / (double)(scores.size());
}

// 并行
double average_score_3(const std::vector<int>& scores) {
  return std::reduce(std::execution::par, scores.begin(), scores.end(), 0) / (double)(scores.size());
}

// 乘积
double scores_product(const std::vector<int>& scores) {
  return std::accumulate(scores.begin(), scores.end(), 1, std::multiplies<int>());
}

void run_score() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v{1, 2, 4, 5, 7, 8, 9};
  std::println("for_loop average {}", average_score(v));
  std::println("std::accumulate average {}", average_score_2(v));
  std::println("std::reduce parallel {}", average_score_3(v));
  std::println("std::accumulate product {}", scores_product(v));
  std::println();
}

// 2.2.2 折叠 (Folding)
// 使用 std::accumulate 计算文件行数 f: (int, char) -> int
int f(int previous_count, char c) {
  return (c != '\n') ? previous_count : previous_count + 1;
}
int count_lines(const std::string& contents) {
  return std::accumulate(contents.begin(), contents.end(), 0, f);
}

// 2.2.3 删除字符串空白符
bool is_not_space(char ch) { return ch != ' '; }
// 值传递
std::string trim_left(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
  return s;
}
std::string trim_right(std::string s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
  return s;
}
std::string trim(std::string s) {
  return trim_left(trim_right(std::move(s)));
}

// 2.2.4 基于谓词分割集合
// partition
struct Person {
  enum class Gender {
    Male,
    Female
  };
  std::string name;
  Gender gender;
};  // struct Person
template <>
// must in namespace std
struct std::formatter<Person> : std::formatter<std::string> {
  auto format(const Person& p, format_context& ctx) const {
    return formatter<string>::format(
      std::format("[name={}, gender={}]", p.name, int(p.gender)), ctx);
  }
};
bool is_female(const Person& person) {
  return person.gender == Person::Gender::Female;
}
void run_partition() {
  PRINT_CURRENT_FUNCTION_NAME;
  using Gender = Person::Gender;
  auto print_person = [](auto person) { std::print("{} ", person); };
  std::vector<Person> people{
    {"Perter", Gender::Male},
    {"Martha", Gender::Female},
    {"Jane", Gender::Female},
    {"David", Gender::Male},
    {"Rose", Gender::Female},
    {"Tom", Gender::Male}
  };
  {
    std::println("all people:");
    std::ranges::for_each(people, print_person);
    std::println();
  }

  auto male_start = std::partition(people.begin(), people.end(), is_female);
  {
    std::println("female:");
    std::for_each(people.begin(), male_start, print_person);
    std::println();
  }
  {
    std::println("male:");
    std::for_each(male_start, people.end(), print_person);
    std::println();
  }
  std::println();
}
// stable_partition
/*
1 2 3 4 5 6 7 8
将目标拖动到 3 和 4 中间
1. 拖动组件 2 到 3 后面, 得到: 1 3 2 4 5 6 7 8
2. 拖动组件 5 到 4 前面, 得到: 1 3 2 5 4 6 7 8
2. 拖动组件 7 到 4 前面, 得到: 1 3 2 5 7 4 6 8
*/
void run_stable_partition() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> vec = std::views::iota(1, 9) | std::ranges::to<std::vector>();
  {
    std::println("all");
    std::ranges::for_each(vec, [](auto n) { std::print("{} ", n); });
    std::println();
  }
  auto destination = vec.begin() + 3;
  {
    std::println("move 2");
    std::stable_partition(vec.begin(), destination, [](auto n) { return n != 2; });
    std::ranges::for_each(vec, [](auto n) { std::print("{} ", n); });
    std::println();
  }
  {
    std::println("move 5");
    std::stable_partition(destination, vec.end(), [](auto n) { return n == 5; });
    ++destination;

    std::ranges::for_each(vec, [](auto n) { std::print("{} ", n); });
    std::println();
  }
  {
    std::println("move 7");
    std::stable_partition(destination, vec.end(), [](auto n) { return n == 7; });
    ++destination;

    std::ranges::for_each(vec, [](auto n) { std::print("{} ", n); });
    std::println();
  }
  std::println();
}

// 2.2.5 过滤 (Filtering) 和转换 (Transforming)
void run_filtering_and_transform() {
  PRINT_CURRENT_FUNCTION_NAME;
  using Gender = Person::Gender;
  auto print_person = [](auto person) { std::print("{} ", person); };
  std::vector<Person> people{
    {"Perter", Gender::Male},
    {"Martha", Gender::Female},
    {"Jane", Gender::Female},
    {"David", Gender::Male},
    {"Rose", Gender::Female},
    {"Tom", Gender::Male}
  };
  {
    std::println("all people");
    std::ranges::for_each(people, print_person);
    std::println();
  }
  {
    std::println("get male by std::remove_if");
    auto tmp_people = people;
    tmp_people.erase(std::remove_if(tmp_people.begin(), tmp_people.end(), is_female), tmp_people.end());
    std::ranges::for_each(tmp_people, print_person);
    std::println();
  }
  {
    auto tmp_people = people;
    std::println("get male by std::ranges::remove_if");
    const auto [first, last]= std::ranges::remove_if(tmp_people, is_female);
    tmp_people.erase(first, last);
    std::ranges::for_each(tmp_people, print_person);
    std::println();
  }
  {
    std::println("get male by std::copy_if");
    decltype(people) males;
    std::copy_if(people.cbegin(), people.cend(), std::back_inserter(males), [](auto& p) { return !is_female(p); });
    std::ranges::for_each(males, print_person);
    std::println();
  }
  {
    std::println("get male by std::ranges::copy_if");
    decltype(people) males;
    std::ranges::copy_if(people, std::back_inserter(males), [](auto& p) { return !is_female(p); });
    std::ranges::for_each(males, print_person);
    std::println();
  }
  {
    std::println("get names by std::transform");
    std::vector<std::string> names(people.size());
    std::transform(people.begin(), people.end(), names.begin(), [](auto& p) { return p.name; });
    std::ranges::for_each(names, [](auto& name) { std::print("{} ", name); });
    std::println();
  }
  {
    std::println("get names by std::ranges::transform");
    std::vector<std::string> names = people | std::views::transform([](auto& p) { return p.name; }) | std::ranges::to<std::vector>();
    std::ranges::for_each(names, [](auto& name) { std::print("{} ", name); });
    std::println();
  }
  std::println();
}

// 2.3 STL 算法的可组合性
// std::transform(std::copy_if(people, is_female), print_person) 性能不高,因为 copy_if 的结果需要临时变量承接,造成不必要的拷贝

// 2.4 编写自己的高阶函数
// 2.4.1 接收函数作为参数
template <typename FilterFunction>
std::vector<std::string> names_for(const std::vector<Person>& people, FilterFunction filter);
// 2.4.2 用循环实现
template <typename FilterFunction>
std::vector<std::string> names_for(const std::vector<Person>& people, FilterFunction filter) {
  std::vector<std::string> result;
  for (const auto& person : people) {
    if (filter(person)) {
      result.emplace_back(person.name);
    }
  }
  return result;
}
// 2.4.3 递归 (Recursion) 和尾调用优化 (Tail-call optimization)
/*
尾递归: 递归后不能做任何事情, 编译器将递归优化成循环(C++标准并不保证一定会进行优化)
*/
/*
1. names_for_2 中没有状态,也就是"纯"函数
2. 问题:
  a. tail 需要拷贝 people, 性能有问题,当然,也可以改成 names_for(begin, end, filter);
  b. prepend 在 vector 最前面插入一个元素,性能也有问题
template <typename FilterFunction>
std::vector<std::string> names_for_2(const std::vector<Person>& people, FilterFunction filter) {
  if (people.empty()) {
    return {};
  } else {
    const auto& head = people.front();
    const auto processed_tail = names_for(tail(people), filter);
    if (filter(head)) {
      return prepend(head.name, processed_tail);
    } else {
      return processed_tail;
    }
  }
}
*/
// 尾递归实现
template <typename FilterFunction, typename Iterator>
std::vector<std::string> names_for_helper(Iterator begin, Iterator end, FilterFunction filter,
    std::vector<std::string> previously_collected) {

  if (begin == end) {
    return previously_collected;
  } else {
    const auto& head  = *begin;
    if (filter(head)) {
      previously_collected.push_back(head.name);
    }
    // 如果没有编译器优化,那么可能会递归爆栈
    return names_for_helper(begin + 1, end, filter, std::move(previously_collected));
  }
}
template <typename FilterFunction, typename Iterator>
std::vector<std::string> names_for_3(Iterator begin, Iterator end, FilterFunction filter) {
  return names_for_helper(begin, end, filter, {});
}

// 2.4.4 使用折叠实现
template <typename Filter>
std::vector<std::string> append_name_if(std::vector<std::string> previously_collected, const Person& p) {
  if (Filter(p)) {
    previously_collected.push_back(p.name);
  }
  return previously_collected;
}
template <typename BeginIt, typename EndIt, typename T, typename F>
T moving_accumulate(BeginIt first, const EndIt& last, T init,
                    F folding_function)
{
    for (; first != last; ++first) {
        init = folding_function(std::move(init), *first);
    }
    return init;
}

template <typename FilterFunction, typename Iterator>
std::vector<std::string> names_for_4(const std::vector<Person>& people, FilterFunction filter) {
  // std::accumulate 按值传递,这样没掉调用 append_name_if 都会拷贝 previously_collected
  // return std::accumulate(people.begin(), people.end(), std::vector<std::string>{}, append_name_if<filter>);
  return moving_accumulate(people.begin(), people.end(), std::vector<std::string>{}, filter);
}


// 练习: 使用 std::accumulate 实现 std::any_if, std::all_of, std::find_if
template <typename InputIterator, typename UnaryPred>
InputIterator find_if(InputIterator first, InputIterator last, UnaryPred p) {
  auto cur = first;
  return std::accumulate(first, last, last, [=, &cur, &p](auto res_it, auto& value) {
    if (res_it != last) {
      return res_it;
    } else if (p(value)) {
      return cur;
    } else {
      ++cur;
      return res_it;
    }
  });
}

template <typename InputIterator, typename UnaryPred>
bool any_of(InputIterator first, InputIterator last, UnaryPred p) {
  return ::find_if(first, last, p) != last;
}

template <typename InputIterator, typename UnaryPred>
bool all_of(InputIterator first, InputIterator last, UnaryPred p) {
  return std::accumulate(first, last, true, [=, &p](auto res, auto& value) {
    return res && p(value);
  });
}

void run_self_define_std() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto p = [](auto n) { return n > 0; };
  {
    std::vector<int> vec;
    auto res = ::find_if(vec.begin(), vec.end(), p);
    assert(res == vec.end());
  }
  {
    std::vector<int> vec{-1, 1, -2};
    auto res = ::find_if(vec.begin(), vec.end(), p);
    assert(*res == 1);
  }
  {
    std::vector<int> vec{-1, -1, -2};
    auto res = ::find_if(vec.begin(), vec.end(), p);
    assert(res == vec.end());
  }
  {
    std::vector<int> vec;
    auto res = ::any_of(vec.begin(), vec.end(), p);
    assert(res == false);
  }
  {
    std::vector<int> vec{-1, 1, -2};
    auto res = ::any_of(vec.begin(), vec.end(), p);
    assert(res == true);
  }
  {
    std::vector<int> vec{-1, -1, -2};
    auto res = ::any_of(vec.begin(), vec.end(), p);
    assert(res == false);
  }
  {
    std::vector<int> vec;
    auto res = ::all_of(vec.begin(), vec.end(), p);
    assert(res == true);
  }
  {
    std::vector<int> vec{-1, 1, -2};
    auto res = ::all_of(vec.begin(), vec.end(), p);
    assert(res == false);
  }
  {
    std::vector<int> vec{-1, -1, -2};
    auto res = ::all_of(vec.begin(), vec.end(), p);
    assert(res == false);
  }
  {
    std::vector<int> vec{1, 1, 2};
    auto res = ::all_of(vec.begin(), vec.end(), p);
    assert(res == true);
  }
  std::println();
}



int main() {
  run_score();
  run_partition();
  run_stable_partition();
  run_filtering_and_transform();
  run_self_define_std();
  return 0;
}


