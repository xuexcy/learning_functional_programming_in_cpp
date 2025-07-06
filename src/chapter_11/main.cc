/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/07/03 14:21:06
# Desc   : 第 11 章 模板元编程
########################################################################
*/

#include <cassert>
#include <list>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "chapter_11/people.h"
#include "common/people.h"
#include "cpp_utils/util.h"

// 11.1 编译时操作类型
template <typename T>
// using ContainedType = decltype(*begin(T()));  // T 可能没有默认构造函数，导致编译失败
using ContainedType = std::remove_cvref_t<decltype(*begin(std::declval<T>()))>;

template <typename C, typename R = ContainedType<C>>
R sum_collection(const C& collection) {
  return std::accumulate(begin(collection), end(collection), R());
}

template <typename C, typename R = C::value_type>
R sum_iterable(const C& collection) {
  return std::accumulate(begin(collection), end(collection), R());
}

SAME_TYPE(int, ContainedType<std::vector<int>>);
SAME_TYPE(std::string, ContainedType<std::list<std::string>>);

void run_sum() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v{1, 2, 3};
  auto r = sum_collection(v);
  std::println("sum={}", r);
  std::println();
}

// 11.1.1 推断类型调试
template <typename, typename...>
struct [[deprecated]] dump{};
dump<ContainedType<std::vector<int>>> d();  // 使用 [[deprecated]], 但是 gcc 的 warning 好像不提示类型

template <typename, typename...>
struct dump2;
// dump2<ContainedType<std::vector<int>>> d2;  // 使用未定义的类，编译出现 error,

// 11.1.2 编译时的模式匹配
template <typename T1, typename T2>
struct is_same : std::false_type {
};  // struct is_same
template <typename T>
struct is_same<T, T> : std::true_type {
};  // struct is_same<T, T>

template <typename T>
struct remove_reference {
  using type = T;
};  // struct remove_reference
template <typename T>
struct remove_reference<T&> {
  using type = T;
};  // struct remove_reference<T&>
template <typename T>
struct remove_reference<T&&> {
  using type = T;
};  // struct remove_reference<T&&>
template <typename T>
using remove_reference_t = remove_reference<T>::type;

// 11.1.3 提供类型的元信息
// 定义 using value_type = T

// 11.2 编译时检查类型的属性
// 接受任意数目的类型，并返回 void
template <typename...>
using void_t = void;
// 参数类型必须是有效类型，如果类型无效，void_t 将触发替代失败的例程，编译器将忽略使用它的定义(忽略,而不是报错)
template <typename C, typename=void_t<>>
struct has_value_type : std::false_type {
};  // struct has_value_type
template <typename C>
struct has_value_type<C, void_t<typename C::value_type>> : std::true_type {
};  // struct has_value_type<C, void_t<typename C::value_type>>
// 如果 C 没有 value_type, 那么 void_t 将会让编译器忽略这个继承 std::true_type 的定义

template <typename C>
auto sum(const C& collection) {
  if constexpr (has_value_type<C>()) {
    return sum_collection(collection);
  } else {
    return sum_iterable(collection);
  }
}

template <typename C, typename = void_t<>>
struct is_iterable : std::false_type {
};  // struct is_iterable

template <typename C>
struct is_iterable<C, void_t<
    decltype(*begin(std::declval<C>())),
    decltype(end(std::declval<C>()))>>: std::true_type {
};  // struct is_iterable

template <typename...>
struct MyFalse : std::false_type {
};  // struct MyFalse
template <typename C>
auto sum2(const C& collection) {
  if constexpr (has_value_type<C>()) {
    return sum_collection(collection);
  } else if constexpr (is_iterable<C>()) {
    return sum_iterable(collection);
  } else {
    static_assert(MyFalse<C>(), "sum can be called only on collections");
  }
}

// 11.3 构造柯里化函数
template <typename Function, typename... CapturedArgs>
class curried {
public:
  curried(Function function, CapturedArgs... args) :
      function_(function), captured_(std::make_tuple(std::move(args)...)) {}
  curried(Function function, std::tuple<CapturedArgs...> args) :
      function_(function), captured_(args) {}

  template <typename... NewArgs>
  auto operator()(NewArgs&&... args) const {
    auto new_args = std::make_tuple(std::forward<NewArgs>(args)...);
    auto all_args = std::tuple_cat(captured_, std::move(new_args));
    if constexpr (std::is_invocable_v<Function, CapturedArgs..., NewArgs...>) {
      // 参数够了，调用函数
      return std::apply(function_, all_args);
    } else {
      // 参数不够，返回绑定新参数后的函数
      return curried<Function, CapturedArgs..., NewArgs...>(function_, all_args);
    }
  }
private:
  Function function_;
  std::tuple<CapturedArgs...> captured_;
};  // class curried

void print_person(const Person& p, std::ostream& out, Person::OutputFormat format) {
  if (format == Person::OutputFormat::NameOnly) {
    std::println(out, "{}", p.name);
  } else if (format == Person::OutputFormat::FullName) {
    std::println(out, "{} {}", p.name, p.surname);
  }
}

// class curried 将参数以值的方式存储，如果需要传递不可复制对象，或因为性能问题想要避免赋值, 可以使用引用包装
// 即 std::cref/std::ref
void run_curried() {
  PRINT_CURRENT_FUNCTION_NAME;
  Person martha{ "Martha" , Gender::Female};

  auto print_person_cd = curried(print_person);
  print_person_cd(std::cref(martha))(std::ref(std::cout))(Person::OutputFormat::NameOnly);

  std::println();
}


// 11.4 DSL 构建块
// 领域特定语言 domain-specific language
/*
with(martha) (
  name = "Martha",
  surname = "Jones",
  age = 42
)

1. 设定 name/surname/age 是封装了 struct Person 的成员变量的指针, 即 auto name = Field {&Person::name}
  以从 'name' 语法得到一个包含 <struct field> 的实例
2. 给 struct Field 提供赋值操作符，得到写的结构 struct Update, 其中记录 name 指针和值
  以从 'name = "Martha"' 得到一个更新操作 <struct field, new value> 的实例
3. 将 with 定义成一个函数，接受 Person 实例，并返回可调用对象，该对象可以接受变长的 Update 实例,
  即返回实例 Transaction{Person}, operator()(Updates...)
4. 最后在 Transaction 中实现 operator(), 为记录更新各个 field

*/
void run_dsl() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    using namespace person;
    Record martha{"martha", "jones", 41};
    // 这里的 name 是 Field{&person::Record::name}
    // 在 struct Update 中执行 operator() 时，调用的语句是 std::invoke(member, record) = value;
    // 因为 Member(即&person::Record::name) 是不能 invoke 的，只能赋值，即 record.name = value
    with(martha)(
      name = "Martha",
      surname = "Jones",
      age = 42
    );
    assert(martha.name == "Martha");
    assert(martha.surname == "Jones");
    assert(martha.age == 42);
  }
  {
    using namespace person2;
    Record martha{"martha", "jones", 41};
    // 这里的 name 是 Field{&person2::Record::set_name}
    // 在 struct Update 中执行 operator() 时，调用的语句是 return std::invoke(member, record, value)
    // 因为 Member(即&person2::Record::set_name) 是可以 invoke 的, 并返回了 bool 值
    with(martha)(
      name = "Martha",
      surname = "Jones",
      age = 42
    );
    assert(martha.name == "Martha");
    assert(martha.surname == "Jones");
    assert(martha.age == 42);
  }
  std::println();
}

int main() {
  run_sum();
  run_curried();
  run_dsl();
  return 0;
}



