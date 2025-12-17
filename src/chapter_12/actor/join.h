/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/28 15:39:07
# Desc   : 12.3.4 连接流
########################################################################
*/
#pragma once

#include <functional>
#include <list>

namespace detail {
template <typename Sender, typename SourceMessageType = Sender::value_type,
    typename MessageType = SourceMessageType::value_type>
class JoinImpl {
public:
  using value_type = MessageType;

  JoinImpl(Sender&& sender) : sender_(std::forward<Sender>(sender)) {}

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    emit_ = std::move(emit);
    sender_.set_message_handler([this](SourceMessageType&& message) {
      process_message(std::forward<SourceMessageType>(message));
    });
  }
  void process_message(SourceMessageType&& source) {
    sources_.emplace_back(std::forward<SourceMessageType>(source));
    sources_.back().set_message_handler(emit_);
  }
private:
  Sender sender_;
  std::function<void(MessageType&&)> emit_;
  std::list<SourceMessageType> sources_;

};  // class JoinImpl

struct JoinHelper {
};  // struct JoinHelper
}

inline auto join() {
  return detail::JoinHelper{};
}

template <typename Sender>
auto join(Sender&& sender) {
  return detail::JoinImpl<Sender>(std::forward<Sender>(sender));
}

template <typename Sender>
auto operator|(Sender&& sender, detail::JoinHelper) {
  return detail::JoinImpl<Sender>(std::forward<Sender>(sender));
}

