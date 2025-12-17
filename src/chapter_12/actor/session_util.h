/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/08/08 16:23:39
# Desc   :
########################################################################
*/
#pragma once

#include <memory>

template <typename SessionType, typename Socket, typename EmitFunction,
    typename SessionImpl = SessionType:: template Impl<EmitFunction>>
std::shared_ptr<SessionImpl> make_shared_session(Socket&& socket, EmitFunction&& emit) {
  return std::make_shared<SessionImpl>(
        std::forward<Socket>(socket),
        std::forward<EmitFunction>(emit));
}
