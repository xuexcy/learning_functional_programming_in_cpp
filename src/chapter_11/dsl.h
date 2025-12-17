/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/05 23:07:48
# Desc   :
########################################################################
*/
#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace dsl {

template <typename Member, typename Value>
struct Update {
  Update(Member _member, Value _value) : member(_member), value(_value) {}

  template <typename Record>
  bool operator()(Record& record) {
    if constexpr (std::is_invocable_r_v<bool, Member, Record, Value>) {
      return std::invoke(member, record, value);
    } else if constexpr (std::is_invocable_v<Member, Record, Value>) {
      std::invoke(member, record, value);
      return true;
    } else {
      std::invoke(member, record) = value;
      return true;
    }
  }

  Member member;
  Value value;
};  // struct Update

// member 是指向类成员的指针
// field(&Person::name) 表示 Person 的 name 域
template <typename Member>
struct Field {
  Field(Member _member) : member(_member) {}
  template <typename Value>
  auto operator=(const Value& value) const {
    // 常见的 operator= 返回的是 *this 的 reference
    return Update{member, value};
  }
  Member member;
};  // struct Field


template <typename Record>
class Transaction {
public:
  Transaction(Record& record) : record_(record) {}
  template <typename... Updates>
  bool operator()(Updates... updates) {
    auto temp = record_;
    if (all(updates(temp)...)) {
      std::swap(record_, temp);  // copy and swap
      return true;
    }
    return false;
  }
private:
  template <typename... Results>
  bool all(Results... results) const {
    return (... && results);
  }
private:
  Record& record_;
};  // class Transaction

template <typename Record>
Transaction<Record> with(Record& record) {
  return Transaction(record);
}

}  // namespace dsl
