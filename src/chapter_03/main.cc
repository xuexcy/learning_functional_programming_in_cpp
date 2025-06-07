/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/06 18:34:35
# Desc   : chapter_03 函数对象
########################################################################
function object
*/

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include "common/people.h"
#include "common/util.h"

#include "boost/phoenix.hpp"
#include "cpp_utils/util.h"

// 3.1 函数和函数对象
// 3.1.1 自动推断返回值类型
// 如果一个函数有多个返回语句,则所有的语句都返回相同的类型
/*
auto task(bool flag) {
  if (flag) return 42;
  else return std::string(42);
}
*/

// 推断出返回值类型后就可以在后续其他地方使用
auto factorial(int n) {
  if (0 == n) { return 1; }  // 这里推断返回值类型为 int
  else { return factorial(n - 1) * n; }  // 这里使用返回值类型 int, 得到 int * n 还是 int
}
// 书上描述: 上面的 factorial 交换 if 和 else 分支就会出错,因为编译器需要先编译 factorial 函数的递归调用
// 之后才是函数的类型推断
// 也就是在编译 if 分支时,还没推断出 factorial 的返回值类型
// 但实际 factorial_2 是可以编译的
auto factorial_2(int n) {
  if (0 != n) { return factorial(n - 1) * n; }
  else { return 1; }
}

// 使用 decltype(auto), 返回值类型式表达式的 decltype 类型
decltype(auto) ask() {
  static int answer{};
  // return answer;  // 表达式类型是 int
  return (answer);  // 表达式类型是 int&
}

/*
1. 如果返回值类型声明为 auto, 那么可能造成不必要的拷贝, 用户也可能要想一个引用
2. 如果返回值类型声明为 auto&, 那么可能引用一个临时变量
使用 decltype(auto) 可以将返回值类型完美传递出来
*/
template <typename Object, typename Function>
decltype(auto) all_on_object(Object&& object, Function function) {
  // 当 function 返回的是值类型, 那 delctype(auto) 就是值类型
  // 当 function 返回的是引用类型, 那 delctype(auto) 就是引用类型
  // 即 decltype(function(...)) 的类型就是 function 的返回类型
  return function(std::forward<Object>(object));
}

// 3.1.2 函数指针
// function pointer: 存放函数地址的变量
int pia() { return 42; }
// using function_ptr = decltype(pia);  // no, 表示函数类型 int()
//typedef decltype(pia) * function_ptr;  // yes
using function_ptr = decltype(&pia);  // yes, 表示函数指针类型 int(*)()
class convertible_to_function_ptr {
public:
  operator function_ptr() const {  // 隐式转换
    return pia;
  }
};  // class convertible_to_function_ptr

void run_function_pointer() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto pia_ptr = &pia;
  std::println("{}", pia_ptr());
  auto& pia_ref = pia;
  std::println("{}", pia_ref());
  convertible_to_function_ptr pia_wrapper;
  std::println("{}", pia_wrapper());
  std::println();
}

// 3.1.3 调用操作符重载
// 除了函数指针,还可以创建一个类,并重载 operator()
// 另外 function_object 还可以拥有状态(成员变量)
class function_object {
public:
 // return_type operator()(arguments) {}
};  // class function_object

bool older_than_42(const Person& p) {
  return p.age > 42;
}
class older_than {
public:
  older_than(int limit): limit_(limit) {}
  bool operator()(const Person& p) {
    return p.age > limit_;
  }
private:
  int limit_;

};  // class older_than
void run_function_object() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<Person> people{
    {"Perter", Gender::Male, 56},
    {"Martha", Gender::Female, 24},
    {"Jane", Gender::Female, 52},
    {"David", Gender::Male, 84},
    {"Rose", Gender::Female, 32},
    {"Tom", Gender::Male, 31}
  };
  std::println("count_if(age > 42): {}", std::count_if(people.begin(), people.end(), older_than_42));
  std::println("older_than(age > 42): {}", std::count_if(people.begin(), people.end(), older_than(42)));
  std::println("older_than(age > 60): {}", std::count_if(people.begin(), people.end(), older_than(60)));
  std::println();
}

// 3.1.4 创建通用函数对象
template <typename T>
class older_than_2 {
public:
  older_than_2(int limit) : limit_(limit) {}
  bool operator()(const T& object) const {
    return object.age > limit_;
  }
private:
  int limit_{};
};  // class older_than_2

struct older_than_3 {
  older_than_3(int limit) : limit_(limit) {}
  template <typename T>
  bool operator()(T&& obj) const {
    return std::forward<T>(obj).age > limit_;
  }
  // 如果不需要修改对象,可以写成 const T&
  // 右值、非const左值，T&& 优先级高于 const T&
  template <typename T>
  bool operator()(const T& obj) const {
    return std::forward<T>(obj).age > limit_;
  }
  int limit_{};
};  // struct older_than_3
void run_template_function_object() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<Person> people{
    {"Perter", Gender::Male, 56},
    {"Martha", Gender::Female, 24},
    {"Jane", Gender::Female, 52},
    {"David", Gender::Male, 84},
    {"Rose", Gender::Female, 32},
    {"Tom", Gender::Male, 31}
  };
  // older_than_2 需要指明模板参数类型 T
  // 不方便
  std::println("count_if(age > 42): {}", std::count_if(people.begin(), people.end(), older_than_2<Person>(42)));
  // 不用指定模板参数, 编译器自动推导
  std::println("count_if(age > 42): {}", std::count_if(people.begin(), people.end(), older_than_3(42)));


  std::println();
}

// 3.2 lambda 和 闭包 (Closure)
/*
1. 使用 lambda，编译器会生成一个 lambda 类，类中可能包含要捕获的引用或值，并且 operator() 是 const 的
  可以在 https://cppinsights.io/ 看到 lambda 捕获的值在生成的类中是 const
  如果需要非 const 的 operator(), 需要将其声明为 mutable
2. 生成 lambda 类后，还会生成一个这个类的实例，也就是闭包, 闭包可以用于存储一些状态
*/
void run_lambda() {
  PRINT_CURRENT_FUNCTION_NAME;
  int count{0};
  std::vector<std::string> words{"An", "ancient", "Pond"};
  std::for_each(words.cbegin(), words.cend(), [count](const std::string& word) mutable {
    // 传递给 std::for_each 的 lambda 函数，就是用它对应的类生成的一个闭包(也就是类实例), 这个闭包中存储了 int count 用于计数
    if (std::isupper(word[0])) {
      std::println("{} {}", word, count);
      // lambda 不是 mutable 的话，这里是不能 ++count 的
      ++count;
    }
  });
  std::println();
}

// 3.2.3 在 lambda 中创建任意成员变量
// 在 [] 中定义并初始化变量，变量类型可以推导
// [session = std::move(session), time = current_time()]() {}

// 3.2.4 通用 lambda 表达式
// lambda 也可以接收 auto 参数, 编译器生成的类，其 operator() 是一个模板函数，就像上面的 struct older_than_3 那样
auto predicate = [limit = 42](auto&& object) {
  return object.age > limit;
};
auto lambda_1 = [](auto first, auto second, auto third, auto fourth) {};
// 当 lambda 有多个参数声明为 auto 时，每个参数的类型都要单独推导，如果参数都属于同一类型，可以使用 decltype 来声明参数类型
auto lambda_2 = [](auto first, decltype(first) second, decltype(first) third, decltype(first) fourth) {};
// cpp20 中可以为 lambda 显示声明模板参数，不再需要 decltype
auto lambda_3 = [] <typename T>(T first, T second, T third, T fourth) {};

// 3.3 编写比 lambda 更简洁的函数对象
struct response_t {
  bool error() const { return error_; }
  int id_;
  bool error_;
};  // struct response_t
template <>
struct std::formatter<response_t> : std::formatter<std::string> {
  auto format(const response_t& res, format_context& ctx) const {
    return formatter<string>::format(
      std::format("[id: {}, error: {}]", res.id_, res.error_), ctx);
  }
};
/*
// 这样写起来比较麻烦，每次都要写一个lambda，而主要逻辑就是判断 response.error() 的值
auto ok_responses = filter(responses, [](const response_t& response) { return !response.error(); });
auto failed_responses = filter(responses, [](const response_t& response) { return response.error(); });
// 期望写成
auto ok_response = filter(responses, not_error);
auto ok_response_2 = filter(responses, !error);
auto ok_response_3 = filter(responses, error == false);
auto failed_response = filter(responses, error);
auto failed_response_2 = filter(responses, error == true);
auto failed_response_3 = filter(responses, not_error == false);
*/

/*
解决办法:
像 struct older_than_3，通过构造函数捕获期望值，并将 operator() 声明为模板函数
*/
class error_test_t {
public:
  error_test_t(bool error = true) : error_(error) {}
  template <typename T>
  bool operator()(T&& value) const {
    return error_ == std::forward<T>(value).error();
  }
  // 支持 error_obj == false
  error_test_t operator==(bool test) const {
    return error_test_t(test ? error_ : !error_);
  }
  // 支持 !error_obj
  error_test_t operator!() const {
    return error_test_t(!error_);
  }
  /*
  注意: 一般的 operator== 和 operator! 都是返回一个 bool 类型，但是我们需要将 == 和 ! 表达式作为一个可调用对象
    传递给 filter, 所以这两个函数也需要返回 error_test_t 的实例
  */
private:
  bool error_;
};  // class error_test_t
error_test_t operator==(bool test, error_test_t error) {
  return  error == test;
}


void run_error() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<response_t> vec{
    {1, true}, {2, false}, {3, false}, {4, true}
  };
  // auto print_responses = [](auto&& responses) {
  //   std::ranges::for_each(responses, [](auto&& res) {
  //     std::print("{} ", res);
  //   });
  //   std::println();
  // };
  auto print_all = []<typename... T>(T&&... responses) {
    (print_container(responses), ...);
  };
  {
    auto ok_responses = std::views::filter(vec, [](const response_t& response) { return !response.error(); });
    auto failed_responses = std::views::filter(vec, [](const response_t& response) { return response.error(); });
    print_all(ok_responses, failed_responses);
  }
  auto error = error_test_t();
  auto not_error = error_test_t(false);
  auto ok_response = std::views::filter(vec, not_error);
  auto ok_response_2 = std::views::filter(vec, error == false);
  auto ok_response_3 = std::views::filter(vec, !error);
  auto ok_response_4 = std::views::filter(vec, not_error == true);
  print_all(ok_response, ok_response_2, ok_response_3, ok_response_4);

  auto failed_response = std::views::filter(vec, error);
  auto failed_response_2 = std::views::filter(vec, error == true);
  auto failed_response_3 = std::views::filter(vec, !not_error);
  auto failed_response_4 = std::views::filter(vec, not_error == false);
  print_all(failed_response, failed_response_2, failed_response_3, failed_response_4);

  std::println();
}

// 3.3.1 STL 中的操作符函数对象
// 3.3.2 其他库中的操作符函数对象
void run_boost_phoenix() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> tmp_numbers{21, 5, 62, 42, 53};
  // gcc 14 不能 print vector
  // std::println("numbers: {}", tmp_numbers);
  print_container(tmp_numbers);
  {
    auto numbers = tmp_numbers;
    // std::less_equal 要求两个参数，我们可以使用 std::bind 来生成一个新的函数对象，但 std::bind 有缺陷(性能问题)
    // std::partition(numbers.begin(), numbers.end(), std::less_equal(42));
    std::partition(numbers.begin(), numbers.end(), std::bind(std::less_equal{}, std::placeholders::_1, 42));
    std::println("numbers after partition (<= 42) using std::less_equal : [{}]", format_container(numbers));
  }

  // 使用 boost::phoenix, 其中 arg1, arg2 等都是 phoenix 中的占位符
  using namespace boost::phoenix::arg_names;
  {
    auto numbers = tmp_numbers;
    // 其中 arg1 <= 42 这个表达式返回一个函数对象，就像前面的 struct error_test_t 中的 operator== 和 operator! 一样
    std::partition(numbers.begin(), numbers.end(), arg1 <= 42);
    std::println("numbers after partition (<= 42) using boost::phoenix::arg_names : [{}]", format_container(numbers));
  }
  {
    auto numbers = tmp_numbers;
    auto res = std::accumulate(numbers.begin(), numbers.end(), 0, arg1 + arg2 * arg2 / 2);
    std::println("numbers accumulate (arg1 + arg2 * arg2 / 2) using boost::phoenix::arg_names : {}", res);
    auto res2{0};
    for (auto& number : numbers) {
      res2 += (number * number / 2);
    }
    std::println("for loop (arg1 + arg2 * arg2 / 2), res: {}", res2);
    assert(res == res2);
    /*
    0 + 21 * 21 / 2 = 220
    220 + 5 * 5 / 2 = 232
    232 + 62 * 62 / 2 = 2154
    2154 + 42 * 42 / 2 = 3036
    3036 + 53 * 53 / 2 = 4440
    */
  }
  std::println();
}

// 3.4 用 std::function 包装函数对象
// 当需要把函数保存下来(比如保存为类成员时), 可以使用 std::function
// std::function 对函数签名进行模板化，而不是对其所包含的类型进行模板化
void run_std_function() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::function<float(float, float)> test_function;
  test_function = std::fmaxf;
  test_function = std::multiplies<float>();
  test_function = std::multiplies<>();
  test_function = [](float a, float b) { return a * b; };
  test_function = [](auto a, auto b) { return a * b; };
  using namespace boost::phoenix::arg_names;
  test_function = (arg1 + arg2) / 2;
}
// std::function 创建了一个对象，需要构造和析构，有性能问题(基于虚成员函数调用，失去了编译器优化的机会)
// 对于函数指针或 std::reference_wrapper 会有小对象优化(SOP small object optimization)
int main() {
  run_function_pointer();
  run_function_object();
  run_template_function_object();
  run_lambda();
  run_error();
  run_boost_phoenix();
  run_std_function();
  return 0;
}
