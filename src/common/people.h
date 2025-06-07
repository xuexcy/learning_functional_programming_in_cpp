/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/06/06 22:42:31
# Desc   :
########################################################################
*/
#pragma once

#include <string>
enum class Gender {
  Male,
  Female
};

struct Person {
  std::string name{};
  Gender gender;
  int age{};
};  // struct Person
