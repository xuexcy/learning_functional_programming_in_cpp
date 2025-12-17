/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/28 14:50:27
# Desc   :
########################################################################
*/
#pragma once

#include <algorithm>
#include <cctype>
#include <locale>

namespace detail {
inline bool is_not_space(char c) {
  return std::isspace(c, std::locale());
}
}

inline std::string trim_left(std::string str) {
  str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), detail::is_not_space));
  return str;
}

inline std::string trim_right(std::string str) {
  str.erase(std::find_if(str.rbegin(), str.rend(), detail::is_not_space).base(), str.end());
  return str;
}
inline std::string trim(std::string str) {
  return trim_left(trim_right(str));
}
