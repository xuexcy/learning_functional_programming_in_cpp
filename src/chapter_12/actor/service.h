/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/07/07 16:38:48
# Desc   :
########################################################################
*/
#pragma once

#include <string>
#include <iostream>

#include "common.h"
#include "session_util.h"
#include "session.h"

template <typename SessionType = Session>
class Service {
public:
  using value_type = SessionType::emit_value_type;
  explicit Service(asio::io_service& service, unsigned short port = 42042) :
      acceptor_(service, tcp::endpoint(tcp::v4(), port)), socket_(service) {}
  Service(const Service&) = delete;
  Service(Service&&) = default;

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    emit_ = emit;
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept(socket_, [this](const error_code& error) {
      if (!error) {
        make_shared_session<SessionType>(std::move(socket_), emit_)->start();
      } else {
        std::println(std::cerr, "{}", error.message());
      }
      do_accept();
    });
  }

private:
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::function<void(value_type&&)> emit_;
};  // class Service
