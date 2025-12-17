/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/07/07 16:00:46
# Desc   : 第 12 章 并发系统的函数式设计
########################################################################
*/

#include <functional>
#include <iostream>

#include "actor/client.h"

#include "actor/filter.h"
#include "actor/join.h"
#include "actor/service.h"
#include "actor/service.h"
#include "actor/session_with_client.h"
#include "actor/sink.h"
#include "actor/transform.h"
#include "actor/values.h"
#include "bookmark/bookmark.h"
#include "utils/common.h"
#include "utils/mbind.h"
#include "utils/mtry.h"
#include "utils/trim.h"

#include "cpp_utils/util.h"



// 12.1 Actor 模型: 组件思想
// 不共享消息，具备接收和发送消息的能力
template <typename SourceMessageType/*接收的消息类型*/, typename MessageType/*发送的消息类型*/>
class Actor {
public:
  using value_type = MessageType;
  void process_message(SourceMessageType&& message);  // 处理接收到的消息 (sink)
  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit);  // 通过 EmitFunction 发送消息 (source)
private:
  std::function<void(MessageType&&)> emit_;
};  // class Actor


auto print_to_cerr = [](const auto& message) {
  std::cerr << message << std::endl;
};

auto not_empty_and_comment = [](const std::string& message) {
  return !message.empty() && message[0] != '#';
};

void run_sink() {
  PRINT_CURRENT_FUNCTION_NAME;
  asio::io_service event_loop;
  auto sink_to_cerr = sink(print_to_cerr);

  auto pipeline1 = sink(Service(event_loop), print_to_cerr);
  auto pipeline2 = Service(event_loop, 42043) | sink_to_cerr;
  auto pipeline3 = Service(event_loop, 42044) | transform(trim) | sink_to_cerr;
  auto pipeline4 = Values{42} | sink_to_cerr;


  auto pipeline5 = Values{42045, 42046, 42047}
    | transform([&](int port) {
      return Service(event_loop, port);
    })
    | join()
    | filter(not_empty_and_comment)
    | sink_to_cerr;

  auto pipeline6 = Service(event_loop, 42048)
    | transform(trim)
    | filter(not_empty_and_comment)
    | transform([](const std::string& message) {
      return mtry([&] {
        return Json::parse(message);
      });
    })
    | filter(&ExpectedJson::has_value)
    | transform([](const ExpectedJson& j) { return j.value(); })
    | transform(bookmark_from_json)
    | filter(&ExpectedBookmark::has_value)
    | transform([](const ExpectedBookmark& b) { return std::format("{}", b.value()); })
    | sink_to_cerr;

  /*
  pipeline6 使用了几个 filter 来处理 Expected, 如果想要忽略错误，可以使用 mbind 来代替
    | filter(&ExpectedJson::has_value)
    | transform([](const ExpectedJson& j) { return j.value(); })
    | transform(bookmark_from_json)
  */
  auto pipeline7 = Service(event_loop, 42049)
    | transform(trim)
    | filter(not_empty_and_comment)
    | transform([](const std::string& message) {
      return mtry([&] {
        return Json::parse(message);
      });
    })
    | transform([](const ExpectedJson& ej) {
      return mbind(ej, bookmark_from_json);
    })
    | filter(&ExpectedBookmark::has_value)
    | transform([](const ExpectedBookmark& b) { return std::format("{}", b.value()); })
    | sink_to_cerr;

  auto pipeline8 = Service(event_loop, 42050)
    | transform(trim)
    | filter(not_empty_and_comment)
    | transform([](const std::string& message) {
      return mtry([&] {
        return Json::parse(message);
      });
    })
    | transform([](const ExpectedJson& ej) {
      return mbind(ej, bookmark_from_json);
    })
    | sink([=](const ExpectedBookmark& eb) {
      if (eb.has_value()) {
        print_to_cerr(std::format("{}", eb.value()));
      }
    });


  // 12.6 响应客户端
  auto pipeline9 = Service(event_loop, 42051)
    | transform(trim)
    | filter(not_empty_and_comment)
    | transform([](const std::string& message) {
      return mtry([&] {
        return Json::parse(message);
      });
    })
    | transform([](const ExpectedJson& ej) {
      return mbind(ej, bookmark_from_json);
    })
    | sink([=](const ExpectedBookmark& eb) {
      if (eb.has_value()) {
        print_to_cerr(std::format("{}", eb.value()));
      }
    });
  event_loop.run();

  std::println();
}
template <typename T>
void print_t(T) {
  PRINT_CURRENT_FUNCTION_NAME;
}

void run_reply() {
  PRINT_CURRENT_FUNCTION_NAME;
  asio::io_service event_loop;
  auto transform_with_client= [](auto&& function) {
    return transform(lift_with_client(std::forward<decltype(function)>(function)));
  };
  auto filter_with_client= [](auto&& function) {
    return filter(apply_with_client(std::forward<decltype(function)>(function)));
  };
  auto t = transform_with_client(trim);
  print_t(t);
  auto pipeline = Service<SessionWithClient>(event_loop, 42050)
      | transform_with_client(trim)
      | filter_with_client(not_empty_and_comment)
      | transform_with_client([](const std::string& message) {
        return mtry([&] {
          return Json::parse(message);
        });
      })
      | transform_with_client([](const ExpectedJson& ej) {
        return mbind(ej, bookmark_from_json);
      })
      | sink([=](auto&& client) {  // Client{ExpectedBookmark, socket}
        auto message = client.value.has_value() ?
            std::format("{}", client.value.value()) :
            std::format("Error processing bookmark");
        print_to_cerr(std::format("This is server: {}", message));
        client.reply(message);
      });
  event_loop.run();

  std::println();
}

int main() {
  //run_sink();
  //run_reply();
  return 0;
}
