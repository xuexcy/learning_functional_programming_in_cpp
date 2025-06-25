/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/23 15:46:35
# Desc   : 第 8 章 函数式数据结构
########################################################################
*/

#include <memory>
#include <print>

#include "cpp_utils/util.h"

// 8.1 不可变链表 (Immutable linked lists)
// 8.1.1 在表头添加和删除元素
struct Node {
  char value{};
  std::shared_ptr<Node> next{nullptr};
  ~Node() {
    // 当 next 自动析构时，会递归析构 next->next, 可能导致堆栈溢出
    // 这里我们主动析构 next
    auto next_node = std::move(next);  // 不使用 move 的话，计数器就会 +1, 导致 use_count 永远不为 0
    while (next_node) {
      if (next_node.use_count() != 1) { break; }  // 如果 count != 1 说明还有其他的 node 的 next 指向这个 next_node

      std::shared_ptr<Node> next_next;
      std::swap(next_next, next_node->next);
      next_node.reset();  // 通过 reset 主动析构当前这个 Node, 前面已经将 next_next swap 出来了，所以不会递归析构

      next_node = std::move(next_next);  // 开始析构下一个 Node, 不用 move 也可以
    }
  }
};  // struct Node

struct List {
  std::shared_ptr<Node> head{nullptr};
  std::shared_ptr<Node> tail{nullptr};
  List prepend(char ch) const {
    auto new_node = std::make_shared<Node>(ch, head);
    return {new_node, tail == nullptr ? new_node : tail};
  }
  List append(char ch) const {
    List res = this->copy();
    auto new_node = std::make_shared<Node>(ch, nullptr);
    if (res.head == nullptr) {
      res.head = new_node;
    } else {
      res.tail->next = new_node;
    }
    res.tail = new_node;
    return res;
  }
  List delete_tail() const {
    return this->copy(tail.get());
  }
  List tail_list() const {
    auto next = head->next;
    if (next != nullptr) {
      return {next, tail};
    } else {
      return {nullptr, nullptr};
    }
  }
  auto all_nodes() const {
    return [this, cur_node=head]() mutable {
      std::shared_ptr<Node> res{nullptr};
      if (cur_node != nullptr) {
        res = cur_node;
        cur_node = cur_node->next;
      }
      return res;
    };
  }
  void print() {
    auto nodes = all_nodes();
    while (auto node = nodes()) {
      std::print("[addr: {}, value: {}] -> ", (void*)node.get(), node->value);
    }
    std::print("[addr: nullptr]");
    std::println();
  }
  List copy(const Node* until=nullptr) const {
    auto pre_head = std::make_shared<Node>();
    auto cur = pre_head;
    auto nodes = all_nodes();
    while (auto node = nodes()) {
      if (node.get() == until) { break; }
      auto new_node = std::make_shared<Node>(node->value);
      cur->next = new_node;
      cur = new_node;
    }
    return {pre_head->next, cur};
  }
};  // struct List

void run_list() {
  PRINT_CURRENT_FUNCTION_NAME;
  List l1;
  auto A = std::make_shared<Node>('A');
  auto B = std::make_shared<Node>('B');
  auto C = std::make_shared<Node>('C');
  auto D = std::make_shared<Node>('D');
  l1.head = A;
  A->next = B;
  B->next = C;
  C->next = D;
  l1.tail = D;

  auto l2 = l1.prepend('E');
  auto l3 = l1.tail_list();
  auto l4 = l1.append('E');
  auto l5 = l1.delete_tail();

  l1.print();
  l2.print();
  l3.print();
  l4.print();
  l5.print();
  std::println();
}


// 8.2 不可变类向量结构
// COW: copy on write

/* bitmapped vector trie/位图向量树，又称 prefix tree 前缀树
向量的向量的向量的。。。的向量, 一层 m 个元素(m 是 2 的指数)
如果最多有 m 个元素，需要一层，即 std::vector<T>, 最大大小为 m
如果最多有 m * m 个元素，需要两层，即 std::vector<std::vector<T>>, 最大大小为 m * m
如果最多有 m * m * m 个元素，需要三层，即 std::vector<std::vector<std::vector<T>>>, 最大大小为 m * m * m
*/
// 8.2.1 位图向量树中的元素查找
// 8.2.2 向位图向量树中追加元素
int main() {
  run_list();
  return 0;
}



