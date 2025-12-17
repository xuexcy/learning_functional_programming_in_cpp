/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/05 23:49:35
# Desc   :
########################################################################
*/
#pragma once

#include <string>

#include "dsl.h"

namespace person {
struct Record {
  std::string name;
  std::string surname;
  unsigned int age;
};  // struct Record

inline auto name = dsl::Field(&Record::name);
inline auto surname = dsl::Field(&Record::surname);
inline auto age = dsl::Field(&Record::age);

}  // namespace person

namespace person2 {
struct Record {
  std::string name;
  std::string surname;
  unsigned int age;
  bool set_name(const std::string& _name) { name = _name; return true; }
  bool set_surname(const std::string& _surname) { surname = _surname; return true;}
  bool set_age(unsigned int _age) { age = _age; return true;}
};  // struct Record

static auto name = dsl::Field(&Record::set_name);
static auto surname = dsl::Field(&Record::set_surname);
static auto age = dsl::Field(&Record::set_age);
}  // namespace person2
