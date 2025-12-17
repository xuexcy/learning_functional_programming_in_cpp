/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/08/04 17:08:44
# Desc   :
########################################################################
*/
#pragma once

#include <expected>

template <typename T, typename E, typename F, typename R = std::invoke_result_t<F, T>>
R mbind(const std::expected<T, E>& exp, F&& f) {
  if (exp.has_value()) {
    return std::invoke(f, exp.value());
  } else {
    return std::unexpected(exp.error());
  }
}
