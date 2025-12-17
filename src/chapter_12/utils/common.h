/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/29 17:03:47
# Desc   :
########################################################################
*/
#pragma once

#include <expected>
#include <type_traits>

#include "nlohmann/json.hpp"

using Json = nlohmann::json;
using ExpectedJson = std::expected<Json, std::exception_ptr>;

template <typename T>
using StoredType = std::conditional_t<
        std::is_lvalue_reference_v<T>,
        T,
        std::remove_reference_t<T>>;

template <typename T>
auto&& get_value(T&& value) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return value;
    } else {
        return std::move(value);
    }
}
