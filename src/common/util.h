/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/06/07 16:53:09
# Desc   :
########################################################################
*/
#pragma once

#include <numeric>
#include <print>

template <typename Container>
auto format_container(Container&& c) {
  return std::accumulate(c.cbegin(), c.cend(), std::string(), [](auto&& pre, auto&& cur) {
    return pre.empty() ? std::format("{}", cur) : std::format("{} {}", pre, cur);
  });
};
template <typename Container>
void print_container(Container&& c) {
  std::ranges::for_each(c, [](auto&& elem) { std::print("{} ", elem); });
  std::println();
};
