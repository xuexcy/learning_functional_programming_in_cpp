/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/29 17:02:26
# Desc   :
########################################################################
*/
#pragma once

#include <exception>
#include <expected>
#include <format>
#include <string>

#include "utils/common.h"
#include "utils/mtry.h"

struct Bookmark {
  std::string url;
  std::string text;
};  // struct Bookmark

template <>
// must in namespace std
struct std::formatter<Bookmark> : std::formatter<std::string> {
  auto format(const Bookmark& b, format_context& ctx) const {
    return formatter<string>::format(
      std::format("Bookmark[url: {}, text: {}]", b.url, b.text), ctx);
  }
};

using ExpectedBookmark = std::expected<Bookmark, std::exception_ptr>;
inline ExpectedBookmark bookmark_from_json(const Json& data) {
  return mtry([&] {
    return Bookmark {
      data.at("FirstUrl"),
      data.at("Text")
    };
  });
}
