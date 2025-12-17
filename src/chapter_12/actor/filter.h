/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/29 16:20:55
# Desc   :  12.4 过滤反应流
########################################################################
*/
#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>

#include "utils/common.h"

namespace detail {
template <typename Sender, typename Predicate, typename MessageType = Sender::value_type>
class FilterImpl {
public:
  using value_type = MessageType;
  using InternalPredicate = std::decay_t<Predicate>;

  FilterImpl(Sender&& sender, Predicate&& predicate) :
      sender_(std::forward<Sender>(sender)), predicate_(std::forward<Predicate>(predicate)) {}
  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    emit_ = std::move(emit);
    sender_.set_message_handler([this](MessageType&& message) {
      this->process_message(std::forward<MessageType>(message));
    });
  }
  void process_message(MessageType&& message) {
    if (std::invoke(predicate_, message)) {
      emit_(std::forward<MessageType>(message));
    }
  }

private:
  Sender sender_;
  InternalPredicate predicate_;
  std::function<void(MessageType&&)> emit_;
};  // class FilterImpl

template <typename Predicate>
struct FilterHelper {
  Predicate predicate;
};  // struct FilterHelper

}  // namespace detail

template <typename Sender, typename Predicate>
auto filter(Sender&& sender, Predicate&& predicate) {
  return detail::FilterImpl<Sender, Predicate>(
    std::forward<Sender>(sender),
    std::forward<Predicate>(predicate)
  );
}

template <typename Predicate>
auto filter(Predicate&& predicate) {
  return detail::FilterHelper<Predicate>(std::forward<Predicate>(predicate));
}

template <typename Sender, typename Predicate>
auto operator|(Sender&& sender, detail::FilterHelper<Predicate> helper) {
  return detail::FilterImpl<Sender, Predicate>(
      std::forward<Sender>(sender),
      get_value<Predicate>(std::move(helper).predicate));
}
