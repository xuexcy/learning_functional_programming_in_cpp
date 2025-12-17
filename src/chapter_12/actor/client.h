/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/08/04 17:33:54
# Desc   :
########################################################################
*/
#pragma once

#include "common.h"
#include "cpp_utils/util.h"

template <typename MessageType>
class Client {
public:
  void reply(const std::string& message) const {
    auto s_ptr = std::make_shared<std::string>(message);
    asio::async_write(*socket, asio::buffer(*s_ptr, s_ptr->length()),
        [s_ptr](auto, auto) {}
    );
  }
public:
  MessageType value;
  tcp::socket* socket;
};  // class Client

// Client{value, socket} -> Client{function(value), socket}
template <typename F>
auto lift_with_client(F&& function) {
  return [function = std::forward<F>(function)] <typename C> (C&& client) {
    return Client{std::invoke(function, std::forward<C>(client).value), client.socket};
  };
}

// Client{value, socket} -> function(value)
template <typename F>
auto apply_with_client(F&& function) {
  return [function = std::forward<F>(function)] <typename C> (C&& client) {
    return std::invoke(function, std::forward<C>(client).value);
  };
}
