/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/19 15:59:44
# Desc   : 第 6 章 惰性求值
########################################################################
*/

#include "cpp_utils/util.h"
#include <map>
#include <mutex>
#include <print>
#include <string>

/*
矩阵乘积
即时求值: auto P = A * B;
  a. bad: 如果不需要计算，就会浪费cpu
函数求值: auto P = [A, B]() { return A * B; };
  a. good: 在需要时调用 P(); 获取结果
  b. bad: 如果值不止被使用一次，那么反复调用 P() 也会浪费 cpu, 最好是将第一次调用的结果存下来

惰性: 不提前，不重复
*/

// 6.1 C++ 的惰性
template <typename F>
class LazyVal {
private:
  F f;
  mutable bool is_evaluated = false;
  using ResultType = decltype(f());
  mutable ResultType value;
  // mutable std::mutex mtx_;
  mutable std::once_flag value_flag_;
public:
  LazyVal(F func): f(func) {}
  operator const ResultType& () const {
    /*
    std::unique_lock lock(mtx_);
    if (!is_evaluated) {
      value = f();
      is_evaluated = true;
    }
    */
    std::call_once(value_flag_, [this](){
      value = f();
    });
    return value;
  }
};  // class LazyVal

template <typename F>
inline LazyVal<F> make_lazy_val(F&& func) {
  return LazyVal<F>(std::forward<F>(func));
}

// 6.2 惰性作为一种优化技术

// 6.2.1 集合惰性排序
// std::nth_element + std::sort

// 6.2.2 用户接口中的列表视图

// 6.2.3 通过缓存函数结果修剪递归树

// 6.2.4 动态编程作为惰性形式
// 将结果缓存到二维数组中
unsigned int lev(const std::string& a, const std::string& b, unsigned int m, unsigned int n) {
  if (m == 0) return n;
  if (n == 0) return m;
  return std::min({
    lev(a, b, m - 1, n) + 1,
    lev(a, b, m, n - 1) + 1,
    lev(a, b, m - 1, n - 1) + (a[m-1] !=b[n-1])
  });
}


// 6.3 通用记忆化 (Generalized memoization)
template <typename Result, typename... Args>
auto make_memoized(Result(*f)(Args...)) {
  std::map<std::tuple<Args...>, Result> cache;
  return [f, cache](Args... args) mutable -> Result {
    const auto args_tuple = std::make_tuple(args...);
    const auto it = cache.find(args_tuple);
    if (it == cache.end()) {
      auto result = f(args...);
      cache[args_tuple] = result;
      return result;
    } else {
      return it->second;
    }
  };
}

auto fib(int n) {
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}

// 使用带有 memoized 的 fib
template <typename F>
unsigned int fib2(F&& fib_memo, unsigned int n) {
  if (n <= 1) return n;
  return fib_memo(n - 1) + fib_memo(n - 2);
}

class NullParam {};
template <typename Sig, class F>
class MemoizeHelper;
template <typename Result, class... Args, class F>
class MemoizeHelper<Result(Args...), F> {
private:
  using FunctionType = F;
  using ArgsTupleType = std::tuple<std::decay_t<Args>...>;
  FunctionType f;
  mutable std::map<ArgsTupleType, Result> cache_;
  mutable std::recursive_mutex cache_mtx_;
public:
  // NullParam 用于和拷贝构造函数区分
  template <typename Function>
  MemoizeHelper(Function&& f, NullParam) : f(f) {}
  MemoizeHelper(const MemoizeHelper& other) : f(other.f) {}

  template <typename... InnerArgs>
  Result operator()(InnerArgs&&... args) const {
    std::unique_lock lock(cache_mtx_);
    const auto args_tuple = std::make_tuple(args...);
    const auto it = cache_.find(args_tuple);
    if (it != cache_.end()) {
      return it->second;
    } else {
      auto&& result = f(*this, std::forward<InnerArgs>(args)...);
      cache_[args_tuple] = result;
      return result;
    }
  }
};  // class MemoizeHelper<Result(Args...), F>

template <typename Sig, class F>
MemoizeHelper<Sig, std::decay_t<F>> make_memoized_helper(F&& f) {
  /*
  这里调用偏特化的 MemoizedHelper 的构造函数, 通过返回类型中模板参数 Sig = Result(Args...) 推导出 Result 和 Args

  既然 MemoizedHelper 没有定义，为什么不直接声明一个和偏特化相同的类型呢?
  template <typename Result, typename... Args, class F>
  class MemoizeHelper {};
  如果像这样声明，那么编译器就无法推导 typename... Args, 比如
  MemoizeHelper<unsigned int, unsigned int, unsigned int>, 最后一个 unsigned int 到底是 Args 还是 F

  如果声明为 template <class F, typename Result, typename... Args>, 那么每次都要显示指定 F 的类型，这样很麻烦,
  所以最好就是通过偏特化，通过函数的返回类型的模板参数 Sig 反向推导出偏特化中的参数 Result 和 Args
  */
  return {std::forward<F>(f), NullParam{}};
}

void run_memoization() {
  PRINT_CURRENT_FUNCTION_NAME;
  { // #1 缓存最终结果

    // fib_memo 只是缓存了最终的计算结果，而无法干预 fib 函数中的调用
    // 也就是 fib_memo 缓存了结果，但 fib 没有使用它
    auto fib_memo = make_memoized(fib);
    std::println("fib(10) = {}", fib_memo(10));
    // 这次的计算会快一点，因为 fib_memo 已经缓存了 fib(10)
    std::println("fib(10) = {}", fib_memo(10));
  }
  { // #2 新建 fib2 函数，在 fib2 中使用 fib_memo 以缓存更多结果

    auto fib_memo = make_memoized(fib);
    // fib2(fib_memo, 10) -> fib_memo(9) -> fib(9) + fib(8), 此时 fib(9) 和 fib(8) 都是没有缓存的
    std::println("fib(10) = {}", fib2(fib_memo, 10));
    // 这次的计算会快一点，因为 fib_memo 已经缓存了 fib(10), fib(9), fib(8)
    // 这样，fib2 比 fib 每次会缓存的数据更多一点
    std::println("fib(10) = {}", fib2(fib_memo, 10));
  }
  { // #3 使用接受可调用对象的类缓存结果

    // 这里 make_memoized_helper 只显示写了第一个模板参数 Sig, 第二个模板参数 F 由函数参数推导得到
    // 这种方式的缓存效率和 #1 中是一样的，都是只缓存最终结果, 只是 #1 只能接受函数指针，#3 可以接受任何可调用对象
    auto fib_memo = make_memoized_helper<unsigned int(unsigned int)>([](auto& fib, unsigned int n) {
      if (n <= 1) return n;
      return fib(n - 1) + fib(n - 2);
      // 上面这么写比较麻烦，因为需要将 fib 的定义重新写一遍，我感觉直接写 return fib(n) 就可以了
    });
    std::println("fib(10) = {}", fib_memo(10));
    std::println("fib(10) = {}", fib_memo(10));
  }
  // #1 和 #3 的
  std::println();
}

// 6.4 表达式模板与惰性字符串拼接
template <typename... Strings>
class LazyStringConcatHelper;
template <typename LastString, typename... Strings>
class LazyStringConcatHelper<LastString, Strings...> {
private:
  LastString data;
  LazyStringConcatHelper<Strings...> tail;
public:
  LazyStringConcatHelper(LastString data, LazyStringConcatHelper<Strings...> tail) :
      data(data), tail(tail) {}
  int size() const {
    return data.size() + tail.size(); // 这个 tail.size() 是递归调用，我们可以用一个 std::size_t size_ 在构造函数中计算好
  }
  template <typename It>
  void save(It end) const {
    const auto begin = end - data.size();
    std::copy(data.cbegin(), data.cend(), begin);
    tail.save(begin);
  }
  operator std::string() const {
    std::string result(size(), '\0');
    save(result.end());
    return result;
  }
  LazyStringConcatHelper<std::string, LastString, Strings...> operator+(const std::string& other) const {
    return LazyStringConcatHelper<std::string, LastString, Strings...>(other, *this);
  }
};  // class LazyStringConcatHelper<LastString, Strings...> {

// 前面的偏特化版本会一直调用下去，我们需要一个特化版本来终止递归
template <>
class LazyStringConcatHelper<> {
public:
  LazyStringConcatHelper() = default;
  int size() const { return 0; }
  operator std::string() const { return {}; }
  LazyStringConcatHelper<std::string> operator+(const std::string& other) const {
    return LazyStringConcatHelper<std::string>(other, *this);  // 这里调用偏特化版本的构造函数
  }
  template <typename It>
  void save(It) const {} // do nothing
};  // class LazyStringConcatHelper<>

LazyStringConcatHelper<> lazy_string;

// 6.4.1 纯洁性与表达式模板
// 在 LazyStringConcatHelper 中存储了 LastString data, 这里如果改成引用，效率会更高，但是这样的惰性求值就不是纯洁的,
// 因为一旦所引用的对象发生改变，同一个 LazyStringConcatHelper 实例得到的结果将会不一样，而且也不安全, 因为所引用的
// 对象可能已经被销毁了
void run_lazy_string() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string name = "Jane";
  std::string surname = "Smith";
  // 一开始调用 operator+ 得到 LazyStringConcatHelper, 最后通过隐式转换 operator std::string() 得到 std::string
  const std::string full_name = lazy_string + surname + ", " + name;
  std::println("Full name: {}", full_name);
  std::println();

}
int main() {
  run_memoization();
  run_lazy_string();
  return 0;
}



