//
// Copyright 2024 Huawei Cloud Computing Technologies Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef OPENGEMINI_COMPLETIONTOKEN_HPP
#define OPENGEMINI_COMPLETIONTOKEN_HPP

#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>

namespace opengemini::token {

class Sync { };

///
/// \~Chinese
/// @brief 该完成令牌使操作阻塞直到任务完成或抛出异常。
///
/// \~English
/// @brief A completion token that causes an operation blocking until the
/// task completes or an exception is thrown.
///
constexpr Sync sync{};

///
/// \~Chinese
/// @brief 该完成令牌使操作返回future对象。
///
/// \~English
/// @brief A completion token that causes an operation to return a future.
///
constexpr auto future = boost::asio::use_future;

///
/// \~Chinese
/// @brief
/// 该完成令牌指示操作应当独立运行，既不感知也不处理任务的结果。
///
/// \~English
/// @brief A completion token that indicate an operation is detached, that is,
/// there is no handler waiting for ther operation's result.
///
constexpr auto detached = boost::asio::detached;

///
/// \~Chinese
/// @brief
/// 该完成令牌指示操作不要立即启动异步任务，而是返回一个函数对象用于懒启动。
///
/// \~English
/// @brief A completion token that specify an operation should return a function
/// object to lazily launch the operation, instead of starting the asynchronous
/// task immediately.
///
constexpr auto deferred = boost::asio::deferred;

#if __cplusplus >= 202002L

///
/// \~Chinese
/// @brief
/// 该完成令牌用于表示当前正在执行的协程。
///
/// \~English
/// @brief A completion token that represent the currently executing coroutine.
///
constexpr auto awaitable = boost::asio::use_awaitable;

#endif // (__cplusplus < 202002L)

} // namespace opengemini::token

#include "opengemini/impl/CompletionToken.tpp"

#endif // !OPENGEMINI_COMPLETIONTOKEN_HPP
