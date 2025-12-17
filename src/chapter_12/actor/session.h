/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/07 16:52:39
# Desc   :
########################################################################
*/
#pragma once

#include <memory>

#include "common.h"

struct Session {
using emit_value_type = std::string;

template <typename EmitFunction>
class Impl : public std::enable_shared_from_this<Impl<EmitFunction>> {
public:
  Impl(tcp::socket&& socket, EmitFunction emit) : socket_(std::move(socket)), emit_(emit) {}
  void start() { do_read(); }
  using SharedSession = std::enable_shared_from_this<Impl<EmitFunction>>;
  void do_read() {
    auto self = SharedSession::shared_from_this();
    asio::async_read_until(socket_, data_, '\n',
        [this, self](const error_code& error, std::size_t size) {
          if (!error) {
            std::istream is(&data_);
            std::string line;
            std::getline(is, line);
            emit_(std::move(line));

            // 开始读取下一条消息
            do_read();
          }
        });
  }
private:
  tcp::socket socket_;
  asio::streambuf data_;
  EmitFunction emit_;
};  // class Impl

};  // struct Session
