/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/17 15:04:58
# Desc   : 第 4 章 以旧函数创建新函数
########################################################################
*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <ranges>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/people.h"
#include "cpp_utils/util.h"

// 4.1 偏函数应用
// partial function application
// 将函数的一个或多个参数设定为特定值，得到新函数
class greater_than {
public:
  greater_than(int value): m_value(value) {}
  bool operator()(int arg) const {
    return arg > m_value;
  }
private:
  int m_value;
};  // class greater_than

// 4.1.1 把二元函数转换成一元函数的通用方法
template <typename Function, typename SecondArgType>
class partial_application_on_2nd_impl {
public:
  partial_application_on_2nd_impl(Function function, SecondArgType second_arg):
      m_function(function), m_value(second_arg) {}
  template <typename FirstArgType>
  decltype(auto) operator()(FirstArgType&& first_arg) const {
    return m_function(std::forward<FirstArgType>(first_arg), m_value);
  }
private:
  Function m_function;
  SecondArgType m_value;
};  // class partial_application_on_2nd_impl

template <typename Function, typename SecondArgType>
partial_application_on_2nd_impl<Function, SecondArgType> bind2nd(
    Function&& f, SecondArgType&& arg) {
  return partial_application_on_2nd_impl<Function, SecondArgType>(
      std::forward<Function>(f), std::forward<SecondArgType>(arg));
}

void print_person(const Person& p, std::ostream& out, Person::OutputFormat format) {
  if (format == Person::OutputFormat::NameOnly) {
    std::println(out, "{}", p.name);
  } else if (format == Person::OutputFormat::FullName) {
    std::println(out, "{} {}", p.name, p.surname);
  }
}

std::vector<Person> people{
  {"Perter", Gender::Male, 56, "A"},
  {"Martha", Gender::Female, 24, "B"},
  {"Jane", Gender::Female, 52, "C"},
  {"David", Gender::Male, 84, "D"},
  {"Rose", Gender::Female, 32, "E"},
  {"Tom", Gender::Male, 31, "F"}
};

void run_partial_function_application() {
  PRINT_CURRENT_FUNCTION_NAME;
  greater_than gt_42(42);
  std::println("1 > 42: {}", gt_42(1));
  std::println("50 > 42: {}", gt_42(50));
  auto gt_42_v2 = bind2nd(std::greater<int>(), 42);

  std::vector<double> degrees = {0, 30, 45, 60};
  std::vector<double> radians(degrees.size());
  std::transform(degrees.cbegin(), degrees.cend(), radians.begin(), bind2nd(std::multiplies<double>(), M_PI/180));
  auto radians_2 = degrees | std::views::transform(bind2nd(std::multiplies<double>(), M_PI/180)) | std::ranges::to<std::vector>();

  // 4.1.2 使用 std::bind 绑定值到特定的函数参数
  auto bound = std::bind(std::greater<double>(), 6, 42);
  assert(false == bound());

  auto is_gt_42 = std::bind(std::greater<double>(), std::placeholders::_1, 42);
  auto is_lt_42 = std::bind(std::greater<double>(), 42, std::placeholders::_1);
  assert(is_lt_42(6));
  assert(!is_gt_42(6));

  // 4.1.3 二元函数参数的反转
  auto lt = std::bind(std::greater<double>(), std::placeholders::_2, std::placeholders::_1);

  // 4.1.4 对多参数函数使用 std::bind
  // std::bind 返回的函数对象中保存的是值的副本，也就是传值 copy，对于 print_person 的第二个参数 std::ostream&
  // 需要使用 std::ref
  std::for_each(people.cbegin(), people.cend(), std::bind(print_person,
      std::placeholders::_1, std::ref(std::cout), Person::OutputFormat::NameOnly));
  std::for_each(people.cbegin(), people.cend(), std::bind(print_person,
      std::placeholders::_1, std::ref(std::cout), Person::OutputFormat::FullName));

  std::for_each(people.cbegin(), people.cend(), [](const Person& p) {
      p.print(std::cout, Person::OutputFormat::NameOnly);
  });
  std::for_each(people.cbegin(), people.cend(), [](const Person& p) {
      p.print(std::cout, Person::OutputFormat::FullName);
  });

  // 4.1.5 使用 lambda 替代 std::bind
  // std::bind 有额外开销
  // lambda 是语言特性，std:bind 标准库，编译器优化语言特性比较容易
  auto bound2 = [] { return std::greater<double>()(6, 42); };
  auto gt_42_v3 = [](double value) { return std::greater<double>()(value, 42); };


  std::println();
}

// 4.2 柯里化(Curring): 看待函数不同的方式
/*
假设编程语言不允许有多于一个参数的函数
1. 需要一个参数，创建一个一元函数 T1
2. 需要两个参数，创建一个一元函数 T1，并返回另一个一元函数 T2 (T1 接受了一个参数，返回的 T2 也可以接受一个参数)
3. 需要三个参数, ... , 一元函数 T2 返回值为另一个一元函数 T3
*/
bool greater(double first, double second) {
  return first > second;
}
// 柯里化版本只接收一个参数，调用后不返回 bool, 而是返回一个一元函数
auto greater_curried(double first) {
  return [first](double second) { return first > second; };
}

auto print_person_cd(const Person& p) {
  return [&](std::ostream& out) {
    return [&](Person::OutputFormat format) {
      print_person(p, out, format);
    };
  };
}

void run_curring() {
  PRINT_CURRENT_FUNCTION_NAME;
  assert(greater(42, 1));
  assert(greater_curried(42)(1));
  /*
  auto print_person_cd = make_curried(print_person);
  print_person_cd(martha, cout, Person::OutputFormat::FullName);
  print_person_cd(martha)(cout, Person::OutputFormat::FullName);
  print_person_cd(martha)(cout)(Person::OutputFormat::FullName);
  print_person_cd(martha, cout)(Person::OutputFormat::FullName);

  auto print_martha = print_person_cd(martha);
  print_martha(cout, Person::OutputFormat::NameOnly);

  auto print_martha_to_cout = print_person_cd(martha, cout);
  print_martha_to_cout(Person::OutputFormat::NameOnly);
  */
  // 4.2.2 数据库访问柯里化
  /*
  声明查询函数 result_t query(connection_t& connection, session_t& session, const std::string& table_name, const std::string& filter)l;
  auto table = "Movies";
  auto filter = "Name = \"Sintel\"";
  auto results = query(local_connection, session, table, filter);

  auto local_query = query(local_connection);
  auto remote_query = query(remove_connection);

  results = local_query(session, table, filter);

  auto main_query = query(local_connection, main_session);

  results = main_query(table, filter);

  auto movies_query = main_query(table);
  results = movies_query(filter);
  */
  std::println();
}

// 4.3 函数组合
// 统计文本中的单词，并按频率大小逆序输出前 n 个频率最高的单词
/*
shell
tr -cs A-Za-z '\n' | tr A-Z a-z | sort | uniq -c | sort -rn | sed ${1}q

c++
1. 打开文本，读取单词
2. 存放到 std::unordered_map<std::string, unsigned int> 中
3. 遍历这个 map, 并将键值对交换 (unsigned int, std::string)
4. 对新的键值对按字典序排序, 先比较数量，再比较单词
*/

std::vector<std::string> words(const std::string& text) {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<std::string> res;
  std::istringstream stream(text);
  std::string word;
  while (stream >> word) {
    res.emplace_back(word);
  }
  return res;
}

template <typename C, typename T = typename C::value_type>
std::unordered_map<T, unsigned int> count_occurrences(const C& collections) {
  PRINT_CURRENT_FUNCTION_NAME;
  std::unordered_map<T, unsigned int> res;
  for (auto& elem : collections) {
    ++res[elem];
  }
  return res;
}

template <typename C,
    typename P = std::pair<
        std::remove_cv_t<typename C::value_type::second_type>,
        std::remove_cv_t<typename C::value_type::first_type>>>
std::vector<P> reverse_pairs(const C& collection) {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<P> res;
  res.reserve(collection.size());
  for (auto& elem : collection) {
    res.emplace_back(elem.second, elem.first);
  }
  return res;
}
template <typename P>
std::vector<P> sort_by_freq(std::vector<P> vec) {
  PRINT_CURRENT_FUNCTION_NAME;
  std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
    return std::tie(a.first, a.second) > std::tie(b.first, b.second);
  });
  return vec;
}

template <typename P>
void print(const std::vector<P>& vec) {
  PRINT_CURRENT_FUNCTION_NAME;
  for(auto& elem : vec) {
    std::println("{} {}", elem.first, elem.second);
  }
}

void print_common_words(const std::string& text) {
  PRINT_CURRENT_FUNCTION_NAME;
  print(sort_by_freq(reverse_pairs(count_occurrences(words(text)))));
}


void run_count_words() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string text = "hello world hello ranges world example text example example";
  print_common_words(text);
  std::println();
}

// 4.4 函数提升(复习)
// 将一个操作元素的函数 f 改成操作元素指针的函数
template <typename Function>
auto pointer_lift(Function f) {
  return [f](auto* item) {
    f(*item);
  };
}
template <typename Function>
auto collection_lift(Function f) {
  return [f](auto& items) {
    for (auto& item : items) {
      f(item);
    }
  };
}

// 4.4.1 键值对列表反转
int main() {
  run_partial_function_application();
  run_curring();
  run_count_words();
  return 0;
}



