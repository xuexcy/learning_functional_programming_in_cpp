/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/25 14:08:47
# Desc   : 第 9 章 代数数据类型及模式匹配
########################################################################
*/

#include <cassert>
#include <cstddef>
#include <expected>
#include <memory>
#include <variant>

#include "cpp_utils/util.h"

/*
start 为 false 时，finished 不能为 true
count > 0 时，start 不能为 false
finished 为 true 时，count 不能被更新
start 为 true, finished 为 false 时，web_page 的端口才应该被打开

但是，struct state_t 并没有如上的限制
*/
struct socket_t {
  std::string url;
  unsigned int port;
};  // struct socket_t
struct state_t {
  bool start;
  bool finished;
  unsigned count = 0;
  socket_t web_page;
};  // struct state_t

// 9.1 代数数据类型
/*
和类型: 将两个类型做和, 得到的值集合是两个类型中值集合的并集，其值不能同时包含两个类型的实例，比如枚举类型 enum, 值
  只能是 enum 中的某个值
积类型: 将两个类型做积，得到的值集合是两个类型中值集合的笛卡尔积，比如 bool 和 bool 做积得到 std::pair<bool, bool>,
  值集合为 <false, false>, <false, true>, <true, false>, <true, true>
*/

/*
state_t 有三种状态
1. 初始状态: 不需要附加信息
2. 计数状态: 需要 count 用于计数，web_page 用于访问
3. 结束状态: 需要最终的计数信息 count
这些状态互斥
*/

// 9.1.1 通过继承实现和类型
// 由于状态是互斥的，所以是 和 类型
class State {
protected:
  State(int type) : type(type) {}
public:
  virtual ~State() {}
  int type;
};  // class State

// 初始状态
class InitState : public State { public:
  enum { id = 0 };
  InitState() : State(id) {}
};  // class InitState

// 运行状态(计数状态)
class RunningState : public State { public:
public:
  enum { id = 1 };
  RunningState() : State(id) {}
  unsigned int count() const {
    return count_;
  }
private:
  unsigned int count_{0};
  socket_t web_page_;
};  // class RunningState

// 结束状态
class FinishedState : public State {
public:
  enum { id = 2 };
  FinishedState(unsigned int count) : State(id), count_(count) {}
  unsigned int count() const { return count_; }
private:
  const unsigned int count_{0};
};  // class FinishedState

class Program {
public:
  Program() : state_(std::make_unique<InitState>()) {}
  void counting_finished() {
    assert(state_->type == RunningState::id);
    // 如果没有 id 来校验状态，那么就需要使用较慢的 dynamic_cast
    auto state = static_cast<RunningState*>(state_.get());
    state_ = std::make_unique<FinishedState>(state->count());
  }
private:
  std::unique_ptr<State> state_;
};  // class Program
/*
通过 class Program
1. 还没有开始统计时，count 不会大于 0(因为 InitState 根本没有 count)
2. 统计结束后 count 也不会被修改(因为 FinishedState 没有提供修改 count 的接口, 另外 count 被声明为 const )
3. 统计结束后，RunningState 中的 web_page 将会自动析构，避免 socket 没有被关闭

使用继承实现和类型
1. 需要使用虚函数
2. 为了避免较慢的强制类型转, 必须使用类标志(也就是 enum { id } )
3. 必须在堆上动态分配内存用于构造对象
*/

// 9.1.2 通过 union 和 std::variant 实现和类型
/*
通过继承的方式，class Program 中的 state_ 可以指向任何 class State 的子类，没有任何限制
1. 使用 std::variant 在定义 state_ 时，必须显式声明所有必须保存在 state_ 中的类型
2. 状态类 InitState 等不再需要继承自 State
3. 各个状态类的实例存放在 std::variant 中(所需内存为最大实例的大小, 像 union 一样), 没有使用 new 来创建实例，并传递指针
*/

class InitState2 {
};  // class InitState2

class RunningState2 {
public:
  RunningState2(const std::string& url) : web_page_(url) {}
  unsigned int count() const { return count_; }
private:
  unsigned int count_;
  socket_t web_page_;
};  // class RunningState2
class FinishedState2 {
public:
  FinishedState2(unsigned int count) : count_(count) {}
  unsigned int count() const { return count_; }
private:
  unsigned int count_;
};  // class FinishedState2

class Program2 {
public:
  Program2() : state_(InitState2()) {}
  void run(const std::string& web_page) {
    assert(0 == state_.index());
    state_ = RunningState2(web_page);
    /*
    ...
    */
    counting_finished();
  }
  void counting_finished() {
    /*
    1. 模板参数类型是类或者index，都可以
    2. std::get_if 返回指针, std::get 返回值或抛出异常
    */
    {
      auto* state1 = std::get_if<1>(&state_);
      assert(state1 != nullptr);
      SAME_TYPE(decltype(state1), RunningState2*);

      auto* state2 = std::get_if<InitState2>(&state_);
      assert(state2 == nullptr);
      SAME_TYPE(decltype(state2), InitState2*);

      auto& state3 = std::get<RunningState2>(state_);
      SAME_TYPE(decltype(state3), RunningState2&);
      assert(&state3 == state1);

      try {
        auto& state4 = std::get<InitState2>(state_);
        SAME_TYPE(decltype(state2), InitState2*);
      } catch (...) {
        std::println("catch exception from std::get<InitState2>");
      }
    }
    auto* state = std::get_if<RunningState2>(&state_);

    assert(state != nullptr);
    state_ = FinishedState2(state->count());
  }
private:
  std::variant<InitState2, RunningState2, FinishedState2> state_;
};  // class Program2

void run_program() {
  PRINT_CURRENT_FUNCTION_NAME;
  Program2 p;
  p.run("google.com");
  std::println();
}

// 9.1.3 特定状态的实现
/*
使用 state_.index() == 0 检查是否是初始状态 InitState2 (在继承方式的 class Program 中我们使用了 id 来判断)
*/

// 9.1.4 特殊的和类型: Optional
struct Nothing {
};  // struct Nothing
template <typename T>
using Optional = std::variant<Nothing, T>;
// 自己实现 get_if
template <typename T, typename Variant>
Optional<T> get_if(const Variant& variant) {
  T* ptr = std::get_if<T>(&variant);
  if (ptr) {
    return *ptr;
  } else {
    return Optional<T>();
  }
}

std::optional<int> a;
// 9.1.5 和类型错误处理
/*
实现细节
1. 构造 T 或 E 的函数
2. 拷贝/移动构造
3. 赋值操作符
4. 析构函数
5. 转换操作符 operator bool(), operator std::optional<T>()
6. get_if 函数，由 Variant 得到 Excepted

*/
template <typename T, typename E>
class Excepted {
private:
  union {
    T value_;
    E error_;
  };
  bool valid_;
public:
  template <typename... Args>
  static Excepted success(Args&&... args) {
    Excepted result;
    result.valid_ = true;
    new (&result.value_) T(std::forward<Args>(args)...);
    return result;
  }
  template <typename... Args>
  static Excepted error(Args&&... args) {
    Excepted result;
    result.valid_ = false;
    new (&result.error_) E(std::forward<Args>(args)...);
    return result;
  }

  Excepted(const Excepted& other) : valid_(other.valid_) {
    if (valid_) {
      new (&value_) T(other.value_);
    } else {
      new (&error_) E(other.error_);
    }
  }
  Excepted(Excepted&& other) : valid_(other.valid_) {
    if (valid_) {
      new (&value_) T(std::move(other.value_));
    } else {
      new (&error_) E(std::move(other.error_));
    }
  }
  // copy and swap 实现赋值
  void swap(Excepted& other) {
    using std::swap;
    if (valid_) {
      if (other.valid_) {
        swap(value_, other.value_);
      } else {
        auto temp = std::move(other.error);

        other.error_.~E();
        new (&other.value_) T(std::move(value_));

        value_.~T();
        new (&error_) E(std::move(temp));

        std::swap(valid_, other.valid_);
      }
    } else {
      if (other.valid_) {
        other.swap(*this);
      } else {
        swap(error_, other.error_);
      }
    }
  }
  Excepted& operator=(Excepted other) {
    swap(other);
    return *this;
  }
  ~Excepted() {
    if (valid_) {
      value_.~T();
    } else {
      error_.~E();
    }
  }
  T& get() {
    if (!valid_) {
      throw std::logic_error("Missing a value");
    }
    return value_;
  }
  E& error() {
    if (valid_) {
      throw std::logic_error("There is no error");
    }
    return error_;
  }
  const T& get() const {
    if (!valid_) {
      throw std::logic_error("Missing a value");
    }
    return value_;
  }
  const E& error() const {
    if (valid_) {
      throw std::logic_error("There is no error");
    }
    return error_;
  }
  operator bool() const {
    return valid_;
  }
  operator std::optional<T>() const {
    if (valid_) { return valid_; }
    else { return std::nullopt; }
  }
};  // class Excepted

template <typename T, typename Variant, typename Excepted = Excepted<T, std::string>>
Excepted get_if(const Variant& variant) {
  T* ptr = std::get_if<T>(variant);
  if (ptr) {
    return Excepted::success(*ptr);
  } else {
    return Excepted::error("Variant doesn't contain the desired type");
  }
}

// 9.2 使用代数数据类型进行域建模

class Tennis {
  enum class Points {
    love,
    fifteen,
    thirty
  };  // enum class Points
  enum class Player {
    player_1,
    player_2,
  };  // enum class Player
  struct NormalScoring {
    Points player_1_points;
    Points player_2_points;
  };  // struct NormalScoring
  struct FortyScoring {
    Player leading_player;
    Points other_player_scores;
  };  // struct FortyScoring
  struct Deuce {
  };  // struct Deuce
  struct Advantage {
    Player player_with_advantage;
  };  // struct Advantage

  void point_for(Player which_player);
  std::variant<NormalScoring, FortyScoring, Deuce, Advantage> state_;
};  // class Tennis

// 9.3 使用模式匹配更好地处理代数数据类型
/*
我们要对 std::variant 中的值进行访问和处理, 可以使用 std::visit
使用 std::visit 对 std::variant 进行处理
std::visit([](const auto& value) {
  std::println("{}", value);
}, variant);
std::visit 第一个参数是一个可调用对象，可调用对象的参数是 variant 的值,

但是我们希望不同的值执行不同的逻辑, 可以使用 if (variant.index() ==0 ) {} else if (variant.index() == 1) {}
这种方式
如果还是想要使用 std::visit 就需要第一个可调用对象参数对 operator() 进行重载，以接受不同类型的值
*/
template <typename... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;  // 这样就得到 sizeof...(Ts) 个重载的 operator()
};  // struct Overloaded
template <typename... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;  // 推导指引

void Tennis::point_for(Player which_player) {
  std::visit(Overloaded {
      [&](const NormalScoring& state) {

      },
      [&](const FortyScoring& state) {

      },
      [&](const Deuce& state) {

      },
      [&](const Advantage& state) {

      }
    },
    state_
  );
}


// 根据 Overloaded 这种实现方式，我们可以为 std::optional, std::excepted 和基于继承的和类型，实现对应的 visit 函数
template <typename F, typename T, typename Opt = std::optional<T>>
void visit(F&& f, Opt&& opt) {
  if (opt.has_value()) {
    f(opt.value());
  } else {
    f(std::nullopt);
  }
}

template <typename F, typename T, typename E, typename Expected= std::expected<T, E>>
void visit(F&& f, Expected&& expected) {
  if (expected.has_value()) {
    f(expected.value());
  } else {
    f(expected.error());
  }
}

// 通过 Overload 重载，可以实现 if-else

// 9.4 Match7 的强大匹配功能
int main() {
  run_program();
  return 0;
}


