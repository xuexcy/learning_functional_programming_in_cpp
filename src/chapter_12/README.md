# some_info
- 只发送消息的 actor 叫做源(source), 需要 `set_message_handler`
- 只接收消息的 actor 叫做宿(sink), 需要在构造函数中就处理源发送来的消息，即 `sender.set_message_handler([](Message message) { emit_(message); })`

# 12.2 创建简单的消息源:
- `class Service`: 仅负责接收消息(`void do_accept()`)
    1. `void set_message_handler(EmitFunction)`: 将处理消息的函数 `EmitFunction` 传递给 `class Service`
    2. `void do_accept()`: 开始连接客户端
        1. 将客户端连接 `socket` 和处理消息的函数 `EmitFunction` 交由 `class Session`: `SharedSession(std::move(socket), emit_function)->start()`
        2. 接着开始接收下一个客户端的消息(再次调用`do_accept()`)
- `class Session`: 处理消息(`void do_read()`)
    1. 使用 `EmitFunction` 处理读取到的消息
    2. 继续读取下一条消息(再次调用 `void do_read()`)

# 12.3 将反应流建模为 monad
- 发送消息接口: `set_message_handler`
## 12.3.1 创建宿 (Sink) 接收消息
-  `class Sink`: 构造函数中处理(`$EMIT`)接收到的消息
```cpp
sender.set_message_handler([](Message message) {
    $Emit(message);
});
```
## 12.3.2 转换反应流
- `class detail::TransformImpl`: 接收消息 -> 转换消息 -> 发送消息
```cpp
sender.set_message_handler([](Message message) {
    $Emit(transformation(message));
});
```
## 12.3.3 创建给定值的流
- `class Values`: 发送初始化列表中的消息
```cpp
for (auto value : values) {
    $Emit(value);
}
```
## 12.3.4 连接流
- `class Join`: 将多个流的消息串联起来
```cpp
sender.set_message_handler([](Source source) {
    sources_.emplace_back(std::move(source));
    sources_.back().set_message_handler($EMIT);
});
```

1. `Values`: `$Emit1` -> `loop[$Emit1(value)]`
2. `Values | transform`: `$Emit2` -> `loop[$Emit2(transformation(value))]`
3. `Values | transform | join()`: `$Emit3` -> `loop[transformation(value).set_message_handler($Emit3)]`
    - 注意，join 会将前一个 Sender 发送的消息保存(`std::move`) 到成员变量 `JoinImpl::source_` 中，这样就不会有生命周期问题
4. `Values | transform | join() | sink(emit)`: 执行 `loop[transformation(value).set_message_handler(emit)]`
```cpp
auto pipeline = Values{42042, 42043, 42044}
    | transform([&](int port) {
        return Service(event_loop, port);
    })
    | join()
    | sink_to_cerr;
```
等价于 `loop[Service(event_loop, port).set_message_handler(sink_to_cerr)]`
```cpp
std::vector<Service> sources;  // 存于 class JoinImpl 中
for (auto port : Values{42042, 42043, 42044}) {
    sources.emplace_back(Service(event_loop, port));
    sources.back().set_message_handler(sink_to_err);
}
```
