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

#include <iostream>
#include <print>
#include <string>

enum class Gender {
  Male,
  Female
};

struct Person {
  std::string name{};
  Gender gender;
  int age{};
  std::string surname{};
  enum class OutputFormat {
    NameOnly,
    FullName
  };  // enum class OutputFormat
  void print(std::ostream& out, OutputFormat format) const {
    if (format == OutputFormat::NameOnly) {
      out << name << std::endl;
    } else if (format == Person::OutputFormat::FullName) {
      out << name << " " << surname << std::endl;
    }
  }
};  // struct Person
