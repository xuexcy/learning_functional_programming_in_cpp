/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/18 20:28:02
# Desc   : 第 5 章 纯洁性: 避免可变状态
########################################################################
*/

#include "common/people.h"
#include "cpp_utils/util.h"
#include <cassert>
#include <list>
#include <numeric>
#include <print>
#include <string>
#include <vector>

// 5.1 可变状态带来的问题
class Movie {
public:
  double average_score() const {
    return std::accumulate(scores.begin(), scores.end(), 0) / (double)scores.size();
  }
private:
  std::string name;
  // 并发读写 scores 并不安全，比如添加新的 score 是在 list 的头还是尾，也会影响 average_score 的计算
  std::list<int> scores;
};  // class Movie

// 5.2 纯函数和引用透明(referential transparency)
/*
将表达式的结果替换整个表达式，程序不会出现不同的行为(比如函数内有一个static的计数器，那么用结果替换表达式就会影响计数)，
那么表达式就是引用透明的,引用透明没有任何可见的副作用，因此表达式中的所有函数都是纯函数

只要函数调用在不改变程序行为的情况下不能使用它的返回值进行替换，那它就有可见的副作用(observable side effect)
*/
double max(const std::vector<double>& numbers) {
  assert(!numbers.empty());
  auto result = std::max_element(numbers.cbegin(), numbers.cend());
  std::println("Maximum is : {}", *result);
  return *result;
}
void run_max() {
  PRINT_CURRENT_FUNCTION_NAME;
  // max 函数不是纯函数，表达式不是引用透明的，如果用 max 的返回值替代函数调用
  // 那么程序将不会执行三次函数调用中的 std::println, 导致没有信息print到标准输出
  auto sum_max = max({1}) +
      max({1, 2}) +
      max({1, 2, 3});
  std::println("{}", sum_max);
  std::println();
}

// 5.3 无副作用编程
enum class Direction {
  Left,
  Right,
  Up,
  Down,
};  // enum class Direction

struct Position {
  Position(unsigned int x, unsigned int y): x{x}, y{y} {
  }
  Position(const Position& original, Direction direction):
      x{direction == Direction::Left ? original.x - 1 :
          direction == Direction::Right ? original.x + 1 : original.x},
      y{direction == Direction::Up? original.y - 1 :
          direction == Direction::Down? original.y + 1 : original.y} {
  }
  unsigned int x{0};
  unsigned int y{0};
};  // struct Position

struct Maze {
  Maze(const std::string& file_path) {
  }
  bool is_wall(const Position& position) const {
    return false;  // 这里返回 false 代表不是墙
  }
  bool is_exit(const Position& position) const {
    return position.x == exit_position_.x && position.y == exit_position_.y;
  }
  Position start_position() const {
    return start_position_;
  }
  Position start_position_{0, 0};
  Position exit_position_{0, 0};
};  // struct Maze

void draw_player(const Position& position, Direction direction) {

}
void draw_maze() {}

Position next_position(Direction direction, const Position& current_position, const Maze& maze) {
  // 计算下一个位置
  auto next_position = Position(current_position, direction);
  return maze.is_wall(next_position) ? current_position : next_position;
}

void process_events(const Maze& maze, const Position& current_position) {
  if (maze.is_exit(current_position)) {
    std::println("exit game, bye bye");
    return;
  }
  const Direction direction{Direction::Down}; // 识别键盘输入
  draw_maze();
  draw_player(current_position, direction);
  const auto new_position = next_position(direction, current_position, maze);
  process_events(maze, new_position);
}
void run_maze() {
  PRINT_CURRENT_FUNCTION_NAME;
  const Maze maze("conf/maze.data");
  // 在这种纯函数的实现方式中，只有 current_position 发生了变化，而且每次都是重新新建一个变量，不需要函数去记住当前位置(状态)
  process_events(maze, maze.start_position());
  std::println();
}

// 5.4 并发环境中的可变状态与不可变状态

// 5.5 const 的重要性
/*
std::string Person::name() const;
相当于
std::string name(const Person* this);  // 其实this最好是一个引用，但是C++引入this的时候还没有引用的概念
*/

// 5.5.1 逻辑 const 与内部 const
// 当成员变量声明为 const 时，编译器就不会生成移动构造函数 move constructor 和移动赋值操作符 move assignment operator
struct Name {
  Name(const std::string& name): name_{name} {}
  Name(const Name&) { PRINT_CURRENT_FUNCTION_NAME; }
  Name(Name&&) { PRINT_CURRENT_FUNCTION_NAME; }
  std::string str() const { return "hi"; }
  std::string name_;
};  // struct Name
class PersonNoneConstMemberVariable {
public:
  PersonNoneConstMemberVariable(const std::string& name): name{name} {}
  Name name;
};  // class PersonNoneConstMemberVariable
class PersonConstMemberVariable {
public:
  PersonConstMemberVariable(const std::string& name): name{name} {}
  const Name name;
};  // class PersonConstMemberVariable

class PersonConstMemberFunction {
public:
  PersonConstMemberFunction(const std::string& name): name_{name} {}
  std::string name() const {
    return name_.str();
  }
private:
  Name name_;
};  // class PersonConstMemberFunction

// 5.5.2 对于临时值优化成员函数
class ConstPerson {
public:
  ConstPerson(const std::string& name) : name_(name) {}
  /*
  const& : 适用于普通值和左值引用，当调用时，拷贝 *this 到 result
  &&: 使用于临时值和右值引用，当调用时，移动 *this 到 result
  */
  ConstPerson with_name(const std::string& name) const& {
    PRINT_CURRENT_FUNCTION_NAME;
    ConstPerson result(*this);  // 拷贝
    result.name_ = name;
    return result;
  }
  ConstPerson with_name(const std::string& name) &&{
    PRINT_CURRENT_FUNCTION_NAME;
    ConstPerson result(std::move(*this));  // 移动
    result.name_ = name;
    return result;
  }
  ConstPerson with_surname(const std::string& surname) const& {
    PRINT_CURRENT_FUNCTION_NAME;
    ConstPerson result(*this);
    result.surname_ = surname;
    return result;
  }
  ConstPerson with_surname(const std::string& surname) &&{
    PRINT_CURRENT_FUNCTION_NAME;
    ConstPerson result(std::move(*this));
    result.surname_ = surname;
    return result;
  }
private:
  std::string name_;
  std::string surname_;
};  // class ConstPerson

// 5.5.4 const 的缺陷
// 1. const 禁止移动对象
ConstPerson some_f() {
  ConstPerson p("Alice");
  /*
  ...
  */
  return p;
}
// 2. Shallow Const
class Company {
public:
  /*
  1. const 函数不允许调用 Company 和 m_employees 的非 const 成员函数, 但是对 m_employees 而言，只是不能修改
    Person* 指针指向的实例，却可以调用 Person 的非 const 成员函数
  2. 如果使用 std::vector<const Person*>, 那么就可以解决第 1 个问题，但是又无法保证 Person 中的成员也是按照此种
    方式使用了 const, 也就是说我们很难检查在当前的 const 成员函数中是否调用了成员的非 const 成员函数
  3. 新的问题: 如果使用 std::vector<const Person*>, 那么如果在其他非 const 成员函数中调用 Person 的非 const 成员函数呢?
  */
  std::vector<std::string> employees_names() const;
private:
  std::vector<Person*> m_employees;
  // std::vector<const Person*> m_employees;
};  // class Company
void run_const() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    PersonNoneConstMemberVariable p1("Alice");
    auto p2 = std::move(p1);  // Name 的移动构造
  }
  {
    PersonConstMemberVariable p1("Alice");
    /*
    std::move(p1) -> std::move(p1.name) -> const Name&&
    构造 const Name name 时使用 Name(Name&&) 构造函数, 由于实参是 const Name&&,
    所以构造时是拷贝而不是移动(对于 const 成员变量，移动构造不会尝试修改它们，而是简单地复制它们的值。)
    */
    auto p2 = std::move(p1);  // Name 的拷贝构造
  }
  {
    ConstPerson p1("Alice");
    // 在 name 中拷贝，在 surname 中移动
    auto new_p = p1.with_name("Bob").with_surname("Smith");
  }
  {
    ConstPerson p1("Alice");
    // 在 surname 中拷贝，在 name 中移动
    auto new_p = p1.with_surname("Smith").with_name("Bob");
  }
  {
    // 1. const 禁止移动对象
    // 如果编译器没有 NRVO, 那么函数中的 return result 将会返回 const ConstPerson, 然后用这个 const 变量来
    // 拷贝构造 p, 而不是移动构造(现在的编译器应该都有 NRVO)
    ConstPerson p = some_f();
  }
}

int main() {
  run_max();
  run_maze();
  run_const();
  return 0;
}



