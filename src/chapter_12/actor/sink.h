/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/08 16:09:38
# Desc   :
########################################################################
*/
#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "utils/common.h"

namespace detail {

// 宿 actor, 接收消息
template <typename Sender, typename Function, typename MessageType = typename Sender::value_type>
class SinkImpl {
public:
  using InternalFunction = std::decay_t<Function>;
  SinkImpl(Sender&& sender, Function&& function) :
      sender_(std::forward<Sender>(sender)), function_(std::forward<Function>(function)) {
    sender_.set_message_handler([this](MessageType&& message) {
      process_message(std::forward<MessageType>(message));
    });
  }
  void process_message(MessageType&& message) const {
    std::invoke(function_, std::forward<MessageType>(message));
  }
private:
  Sender sender_;
  InternalFunction function_;
};  // class SinkImpl

template <typename Function>
struct SinkHelper {
  Function function;
};  // struct SinkHelper

}  // namespace detail

template <typename Sender, typename Function>
auto sink(Sender&& sender, Function&& function) {
  return detail::SinkImpl<Sender, Function>(
    std::forward<Sender>(sender),
    std::forward<Function>(function)
  );
}
template <typename Function>
auto sink(Function&& function) {
  return detail::SinkHelper<Function>(
    std::forward<Function>(function)
  );
}

template <typename Sender, typename Function>
auto operator|(Sender&& sender, detail::SinkHelper<Function> sink) {
  return detail::SinkImpl<Sender, Function>(
      std::forward<Sender>(sender),
      get_value<Function>(std::move(sink).function));
}

