/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/28 14:57:35
# Desc   : 第 10 章 monad
########################################################################
*/

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <exception>
#include <expected>
#include <optional>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "cpp_utils/util.h"
#include "range/v3/all.hpp"
#include "range/v3/view/for_each.hpp"

// 10.1 仿函数并不是以前的仿函数
/*
c++ 中的 functor: 一个带有调用操作符的类 a class with the call operator
函数式编程中的仿函数 functor:
  a. 一个类模板 F 包含一个 transform 函数
  b. transform 函数接受 F<T1> 的实例和函数 t: T1->T2, 返回一个 F<T2> 类型的值
  则 F 是一个仿函数
*/


template <typename T>
class F {
    T value;
public:
    explicit F(const T& v) : value(v) {}

    // transform 函数
    template <typename Func>
    auto transform(Func&& f) const {
        using ResultType = std::invoke_result_t<Func, T>;
        return F<ResultType>(f(value));
    }

    const T& get() const { return value; }
};

void run_functor() {
  PRINT_CURRENT_FUNCTION_NAME;
  F<int> t1(1);
  auto t2 = t1.transform([](int x) {
    return std::to_string(x);
  });
  SAME_TYPE(decltype(t2), F<std::string>);
  assert(t2.get() == "1");
  std::println();
}

template <template<typename> class F, typename T, typename Func>
concept Functor = requires(F<T> ft, Func f) {
  { ft.transform(f) } -> std::same_as<F<std::invoke_result_t<Func, T>>>;
};

static_assert(Functor<F, int, decltype([](int x) {
  return std::to_string(x);
})>);

// 10.1.1 处理可选值
// 给 std::optional 类型定义一个转换函数，那它是一个基本的仿函数
template <typename T1, typename F>
std::optional<std::invoke_result_t<F, T1>> transform(const std::optional<T1>& opt, F f) {
  if (opt.has_value()) {
    return std::make_optional(f(opt.value()));
  } else {
    return std::nullopt;
  }
}
// 让 std::optional 支持管道操作符
// 将 std::optional 转换成一个 std::ranges::subrange
template <typename T>
auto to_range(const std::optional<T>& opt) {
  static constexpr std::array<T, 0> kEmpty;
  return opt.has_value() ? std::ranges::subrange(&opt.value(), (&opt.value()) + 1) :
      std::ranges::subrange(kEmpty.begin(), kEmpty.end());
}

void run_optional() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto f = [](int x) { return std::to_string(x); };
  {
    std::optional<int> opt(1);
    auto opt2 = transform(opt, f);
    assert(opt2.has_value());
    assert("1" == opt2.value());
  }
  {
    std::optional<int> opt(1);
    auto opt_view = to_range(opt);
    SAME_TYPE(decltype(opt_view), (std::ranges::subrange<const int*, const int*>));
    auto opt2 = opt_view | std::views::transform(f);
    for (auto v : opt2) {
      std::println("{}", v);
    }
  }
  {
    std::optional<int> opt{};
    auto opt2 = to_range(opt) | std::views::transform(f);
    for (auto v : opt2) {
      std::println("{}", v);
    }
  }
  auto get_user_name = [](bool state) { return "user_name"; };
  auto to_html = [](const std::string& user_name) { return std::format("html<{}>", user_name); };
  /*
  1. 登录状态 -> get_user_name(state) -> to_html(user_name)
  2. 未登录状态 -> do_nothing
  函数调用 to_html(get_user_name(state)), 如果用户未登录，那么就不能这么写，只能写 if-else
  如果使用 std::optional + transform 或
    to_range(std::optional) + std::view::transform, 就不用考虑用户未登录的状态
  */
  {
    std::optional<bool> state;
    auto html = transform(transform(state, get_user_name), to_html);
    assert(!html.has_value());
    state = true;
    html = transform(transform(state, get_user_name), to_html);
    assert(html.has_value());
    std::println("html: {}", html.value());
  }
  {
    std::optional<bool> state;
    auto html = to_range(state) | std::views::transform(get_user_name) | std::views::transform(to_html);
    assert(html.empty());
    state = true;
    html = to_range(state) | std::views::transform(get_user_name) | std::views::transform(to_html);
    assert(!html.empty());
    for (auto elem : html) {
      std::println("html: {}", elem);
    }
  }
}
// 同样地，也可以对 std::expected<T, E> 实现 transform 或者 to_range

// 10.2 monad: 更强大的仿函数
/*
对于 get_user_name 和 to_html, 如果返回类型是 std::optional, 那么多次 transform 后，得到的就是多层嵌套的值，即
std::optional<std::optional<std::optional<...<std::string>...>>>

我们前面看到的仿函数有一个transform: F<T1> -> F<T2>, 如果 F 是 std::optional, 函数是返回 std::optional<std::string> 的
get_username, 那么调用 transform 后, 就会得到 std::optional<std::optional<std::string>>

monad M<T> 是是一个定义了附加函数的仿函数，用于去除嵌套
join: M<M<T>> -> M<T>

*/
std::optional<std::string> get_user_name(bool login_state) {
  if (login_state) { return "user_name"; }
  else { return std::nullopt; }
}
std::optional<std::string> to_html(const std::string& user_name) {
  if (!user_name.empty()) {
    return std::format("html<{}>", user_name);
  } else {
    return std::nullopt;
  }
}

template <typename T>
std::optional<T> join(std::optional<std::optional<T>> opt) {
  if (opt.has_value()) {
    return opt.value();
  } else {
    return std::nullopt;
  }
}


void run_join() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    std::optional<bool> state;
    auto html = join(transform(
        join(transform(state, get_user_name)),
        to_html));
    assert(!html.has_value());
  }
  {
    std::optional<bool> state{true};
    auto html = join(transform(
        join(transform(state, get_user_name)),
        to_html));
    assert(html.has_value());
    assert("html<user_name>" == html.value());
  }
  {
    using ToRange = decltype(&to_range<std::string>);
    std::optional<bool> state;
    auto login_as_range = to_range(state);
    auto html = login_as_range
        | std::views::transform(get_user_name)
        | std::views::transform(static_cast<ToRange>(to_range<std::string>))
        | std::views::join
        | std::views::transform(to_html)
        | std::views::transform(static_cast<ToRange>(to_range<std::string>))
        | std::views::join;
  }
  std::println();
}

/*
monad M
1. construct : T -> M<T>
2. mbind: (M<T1>, T1 -> M<T2>) -> M<T2>

monad 条件
1. f: T1 -> M<T2>, 则 mbind(construct(a), f) == f(a)
2. mbind(m, construct) == m.
  证明:
    令 m = construct(a), f = construct, 则 mbind(construct(a), f) == f(a) , 替换 construct(a) 和 f,
    则推导出 mbind(m, construct) == construct(a) == m
3. mbind(mbind(m, f), g) == mbind(m, [](auto x) { return mbind(f(x), g); })
  设 lambda = [](auto x) { return mbind(f(x), g); }
  证明: mbind(mbind(m, f), g) == mbind(m, lambda)
    令 m = construct(x),
    1. 等式左边，mbind(m, f) == mbind(construct(x), f) == f(x), 得到
      mbind(mbind(m, f), g) == mbind(f(x), g)
    2. 等式右边, mbind(m, lambda) = mbind(construct(x), lambda) = lambda(x) = mbind(f(x), g)
    得证等式左右边都为 mbind(f(x), g)

对比仿函数 F 和 monad M, 主要区别在于接受的函数
1. 仿函数 F 中 transform 接受的函数是 T1 -> T2, 从实例 F<T1> 得到 F<T2> 实例
  monad M 中 mbind 接受的函数是 T1 -> M<T2>, 从实例 M<T1> 得到 M<T2> 实例
  比如我们之前的 get_user_name 和 to_html, 是 std::string -> std::optional<std::string>
  如果使用 F, 则初始实例为 F<std::string>,
    1. 调用一次transform得到 F<std::optional<std::string>>, 其中 T1 = std::string, T2 = std::optional<std::string>
    2. 再调用一次transform就得到 F<std::optional<std::optional<std::string>>>, 其中 T1 = std::optional<std::string>, T2 = std::optional<std::optional<std::string>>
  如果使用 M, 则初始实例为 M<std::string>,
    1. 调用一次mbind得到 M<std::string>, 其中 T1 = std::string, T2 = std::string
    2. 再调用一次mbind得到 M<std::string>, 其中 T1 = std::string, T2 = std::string
2. monad M 中有一个 construct 函数, 因为我们需要知道

*/

// 10.3 基本的例子
template <typename T>
auto to_vector(T&& value) {
  using ElementType = typename std::remove_cvref_t<T>::value_type;
  std::vector<ElementType> vec{value.size()};
  for (auto&& e : value) {
    vec.emplace_back(e);
  }
  return vec;
}

// 转换一个 vector
template <typename T, typename F>
auto transform(const std::vector<T>& xs, F f) {
  return xs | std::views::transform(f) | std::ranges::to<std::vector>();
}

template <template <typename>typename C, typename T>
auto construct(T&& value) {
  return value | std::views::all | std::ranges::to<C>();
}

template <typename T, typename F>
auto mbind(const std::vector<T>& xs, F f) {
  return xs
      | std::views::transform(f)
      | std::views::join
      | std::ranges::to<std::vector>();
}

template <std::ranges::range Range, typename F>
decltype(auto) mbind(Range&& range, F f) {
  return range
      | std::views::transform(f)
      | std::views::join;
}

namespace detail {
template <typename F>
struct MbindHelper {
  F f;
};  // struct MbindHelper
}  // namespace detail

template <typename F>
detail::MbindHelper<F> mbind(F&& f) {
  return { std::forward<F>(f) };
}

/*
template <typename T, typename F>
auto operator|(const std::vector<T>& vec, F&& f);

1. 使用 detail::MbindHelper 而不是 F 直接作为 operator| 的参数，可以防止和其他 operator| 重载冲突
2. MbindHelper 可以对 F 延迟求值，特别是当 F 很复杂时，如果直接使用 F 就会直接求值
3. 参数 F&& f 的类型需要推导，如果使用 MbindHelper 就可以由 MbindHelper 直接指定, 而不需要operator| 推导
  模板参数 F 和函数参数 F&& f 是两回事，模板参数 F 是可以指定的，但是函数参数如果是像右值引用一样的 T&& 、F&& 就是需要推导的
  但是 MbindHelper<F> 是不需要 operator| 推导的 (只有带有 '模板参数&&' 要推导)
4. 通过 MbindHelper 可以存储 f 的副本，避免超出 f 的生命周期
5. 使用 mbind(f) 返回一个 helper 更加符合链式调用
*/
template <typename T, typename F>
auto operator|(const std::vector<T>& vec, const detail::MbindHelper<F>& helper) {
  return mbind(vec, helper.f);
}
template <std::ranges::range Range, typename F>
auto operator|(Range&& range, const detail::MbindHelper<F>& helper) {
  return mbind(std::forward<Range>(range), helper.f);
}

void run_example() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    std::vector<int> v{1, 2, 3};
    auto v2 = transform(v, [](auto x) { return std::to_string(x); });
    const std::vector<std::string> expected = {"1", "2", "3"};
    assert(expected == v2);
  }
  {
    // construct 用 vector 中的元素构造成一个新的 vector
    std::vector<std::string> v{"abc", "def", "gh"};
    auto v2  = mbind(v, [](auto&& s) {
      return construct<std::vector>(std::forward<decltype(s)>(s));
    });
    for (auto&& e : v2) {
      std::print("{} ", e);
    }
    std::println();
  }
}

// 10.4 range 与 monad 的嵌套使用
// monad 相当于 transform 和 join 的组合

// 使用 mbind 实现 filter
template <typename C, typename P>
auto filter(const C& collection, P predicate) {
  return collection | mbind([=](auto element) {
    return std::views::single(element) | std::views::take(predicate(element) ? 1 : 0);
  });
}

void run_filter() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v{1, 2, 3, 4};
  auto v2 = filter(v, [](auto e) { return e % 2; });
  std::println("{}", v2);
  {
    auto res = std::views::iota(1)
        | mbind([] (int z) {
          return std::views::iota(1, z)
              | mbind([z](int y) {
                  return std::views::iota(y, z)
                      | std::views::transform([y, z](int x) {
                        return std::make_tuple(x, y, z);
                      });
              });
        })
        | std::views::filter([](auto&& triple) {
          auto& [x, y, z] = triple;
          return (x * x + y * y == z * z);
        });
    std::size_t cnt{0};
    for (auto&& elem : res) {
      ++cnt;
      std::println("{}", elem);
      if (10 == cnt) { break; }
    }
  }
  {
    // 前面使用 mbind +  filter 效率比较低，因为前面的 mbind 会先生成所有的(x, y, z) 组合的视图给到 filter
    // 如果在 mbind 中通过 range-v3 的 yield_if，就可以将不符合条件的值不放到生成的视图中
    auto res = std::views::iota(1) | std::views::transform([](int z) {
      return std::views::iota(1, z) | std::views::transform([z](int y) {
        return std::views::iota(y, z) | std::views::transform([y, z](int x) {
          return ranges::yield_if(x * x + y * y == z * z, std::make_tuple(x, y, z));
        }) | std::views::join;
      }) | std::views::join;
    }) | std::views::join;
    std::size_t cnt{0};
    for (auto&& elem : res) {
      ++cnt;
      std::println("{}", elem);
      if (10 == cnt) { break; }
    }
  }
  std::println();
}

// 10.5 错误处理
// 10.5.1 std::optional<T> 作为 monad
// 当使用 std::optional<T> 时，代码可能出现大量的 if (opt.has_value()) 的判断
template <typename T, typename F>
auto mbind(const std::optional<T>& opt, F f) {
  using Ret = std::invoke_result_t<F, T>;
  if (opt) { return f(opt.value()); }
  else { return Ret{}; }
}
template <typename T, typename F>
auto operator|(const std::optional<T>& opt, const detail::MbindHelper<F>& helper) {
  return mbind(opt, helper.f);
}

void run_mbind_for_optional() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    const std::optional<bool> login_state;
    auto html = login_state | mbind(get_user_name) | mbind(to_html);
    assert(!html.has_value());
  }
  {
    const std::optional<bool> login_state(true);
    auto html = login_state | mbind(get_user_name) | mbind(to_html);
    assert("html<user_name>" == html.value());
  }
  std::println();
}

std::expected<std::string, int> get_user_name_v2(bool login_state) {
  if (login_state) { return "user_name"; }
  else { return std::unexpected(-1); }
}
std::expected<std::string, int> to_html_v2(const std::string& user_name) {
  if (!user_name.empty()) {
    return std::format("html<{}>", user_name);
  } else {
    return std::unexpected(-2);
  }
}
// 10.5.2 expected<T, E> 作为 monad
template <typename T, typename E, typename F>
auto mbind(const std::expected<T, E>& exp, F f) {
  using Ret = typename std::invoke_result_t<F, T>;
  if (!exp.has_value()) {
    return Ret(std::unexpected(exp.error()));
  } else {
    return f(exp.value());
  }
}
template <typename T, typename E, typename F>
auto operator|(const std::expected<T, E>& exp, const detail::MbindHelper<F>& helper) {
  return mbind(exp, helper.f);
}
void run_mbind_for_expected() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    std::expected<bool, int> login_state{std::unexpect};
    auto html = login_state | mbind(get_user_name_v2) | mbind(to_html_v2);
    assert(!html.has_value());
  }
  {
    std::expected<bool, int> login_state{true};
    auto html = login_state | mbind(get_user_name_v2) | mbind(to_html_v2);
    assert("html<user_name>" == html.value());
  }
  std::println();
}

// 10.5.3 try monad
template <typename F, typename Ret = typename std::invoke_result_t<F>, typename Exp = std::expected<Ret, std::exception_ptr>>
Exp mtry(F f) {
  try {
    return Exp(f());
  } catch (...) {
    return Exp(std::unexpected(std::current_exception()));
  }
}

void run_monad_try() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    auto f = []() { return 1; };
    auto result = mtry(f);
    assert(1 == result);
  }
  {
    auto f_except = []() -> int { throw std::runtime_error("f_except error"); };
    auto result = mtry(f_except);
    assert(!result.has_value());
    try {
      std::rethrow_exception(result.error());
    } catch (const std::exception& e) {
      std::println("{}", e.what());
    } catch (...) {
      std::println("uncaught");
    }

  }

  std::println();
}

// 10.6 monad 状态处理
// 纯函数不能改变状态，为了记录状态，可以将数据+状态封装成一个新的结构，每次调用函数时将状态进行合并保留
template <typename T>
class WithLog {
public:
  WithLog(T value, std::string log = std::string()): value_(value), log_(log) {}
  T value() const { return value_; }
  std::string log() const { return log_; }
private:
  T value_;
  std::string log_;
};  // class WithLog

template <typename T, typename F, typename Ret = typename std::invoke_result_t<F, T>>
Ret mbind(const WithLog<T>& val, F f) {
  const auto result_with_log = f(val.value());
  return Ret(result_with_log.value(), val.log() + result_with_log.log());
}

// 10.7 并发和延续 monad
// 10.7.1 future 作为 monad
/*
future 是一个仿函数
1. 接收 future<T1>
2. 接受函数 f: T1 -> T2
3. 返回 future<T2>
*/
// 10.7.2 future 的实现
// c++ 标准库不支持 future then

// 10.8 monad 组合
template <typename F, typename G>
auto mcompose(F f, G g) {
  return [=](auto value) {
    return mbind(f(value), g);
  };
}

void run_compose() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto user_html = mcompose(get_user_name, to_html);
  {
    std::optional<bool> login_state;
    auto html = login_state | mbind(user_html);
    assert(!html.has_value());
  }
  {
    std::optional<bool> login_state{true};
    auto html = login_state | mbind(user_html);
    assert("html<user_name>" == html.value());
  }
  std::println();
}

int main() {
  run_functor();
  run_optional();
  run_join();
  run_example();
  run_filter();
  run_mbind_for_optional();
  run_mbind_for_expected();
  run_monad_try();
  run_compose();
  return 0;
}




