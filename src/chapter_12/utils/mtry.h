/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/31 13:49:25
# Desc   :
########################################################################
*/
#pragma once

#include <exception>
#include <expected>
#include <type_traits>

template <typename F, typename Ret = typename std::invoke_result_t<F>, typename Exp = std::expected<Ret, std::exception_ptr>>
Exp mtry(F f) {
  try {
    if constexpr (std::is_void_v<Ret>) {
      f();
      return Exp();
    } else {
      return Exp(f());
    }
  } catch (...) {
    return Exp(std::unexpected(std::current_exception()));
  }
}
