/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/28 15:06:09
# Desc   :
########################################################################
*/
#pragma once

#include <functional>
#include <initializer_list>
#include <vector>

// 12.3.3 创建给定值的流
// 源 actor, 发送消息
template <typename T>
class Values {
public:
  using value_type = T;
  explicit Values(std::initializer_list<T> values) : values_(values) {
  }

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    emit_ = std::move(emit);
    for (auto value : values_) {
      emit_(std::move(value));
    }
  }

private:
  std::vector<T> values_;
  std::function<void(T&&)> emit_;
};  // class Values

