/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/28 14:32:39
# Desc   :
########################################################################
*/
#pragma once

#include "cpp_utils/util.h"
#include <functional>
#include <type_traits>
#include <utility>

// 12.3.2 转换反应流
// 使用 Transformation 将 SourceMessageType 转换为 MessageType
// 接收消息 -> 转换消息 -> 发送消息
namespace detail {
template <typename Sender, typename Transformation,
    typename SourceMessageType = typename Sender::value_type,
    typename MessageType = std::invoke_result_t<Transformation, SourceMessageType>>
class TransformImpl {
public:
  using value_type = MessageType;
  using InternalTransformation = std::decay_t<Transformation>;
  TransformImpl(Sender&& sender, Transformation&& transformation) :
      sender_(std::forward<Sender>(sender)),
      transformation_(std::forward<Transformation>(transformation)) {}

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    emit_ = std::move(emit);
    sender_.set_message_handler([this](SourceMessageType&& message) {
      process_message(std::forward<SourceMessageType>(message));
    });
  }
  void process_message(SourceMessageType&& message) {
    emit_(std::invoke(transformation_, std::forward<SourceMessageType>(message)));
  }
private:
  Sender sender_;
  InternalTransformation transformation_;
  std::function<void(MessageType&&)> emit_;
};  // class TransformImpl

template <typename Transformation>
struct TransformHelper {
  Transformation function;
};  // struct TransformHelper
}  // namespace detail

template <typename Sender, typename Transformation>
auto transform(Sender&& sender, Transformation&& transformation) {
  return detail::TransformImpl<Sender, Transformation>(
    std::forward<Sender>(sender),
    std::forward<Transformation>(transformation)
  );
}

template <typename Transformation>
auto transform(Transformation&& transformation) {
  return detail::TransformHelper<Transformation>(
    std::forward<Transformation>(transformation)
  );
}

template <typename Sender, typename Transformation>
auto operator|(Sender&& sender, detail::TransformHelper<Transformation> transform) {
  return detail::TransformImpl<Sender, Transformation>(
    std::forward<Sender>(sender),
    get_value<Transformation>(std::move(transform).function)
  );
}

