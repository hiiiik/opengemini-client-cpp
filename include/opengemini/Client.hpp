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

#ifndef OPENGEMINI_CLIENT_HPP
#define OPENGEMINI_CLIENT_HPP

#include <memory>

#include "opengemini/ClientConfig.hpp"
#include "opengemini/CompletionToken.hpp"

namespace opengemini {

namespace impl {
class ClientImpl;
}

///
/// \~Chinese
/// @brief openGemini客户端。
///
/// \~English
/// @brief An openGemini client.
///
class Client {
public:
    ///
    /// \~Chinese
    /// @brief 构造函数。
    /// @details 创建一个能与openGemini数据库集群通讯的客户端。
    /// @param config 客户端配置，可以直接构造该结构体 @ref ClientConfig
    /// ，或通过 @ref ClientConfigBuilder 构建。
    ///
    /// \~English
    /// @brief A constructor.
    /// @details Creates a client that can be used to communicate with the
    /// openGemini cluster.
    /// @param config The client's configuration, which can be directly
    /// constructed as @ref ClientConfig structure, or can be built using @ref
    /// ClientConfigBuilder.
    ///
    explicit Client(const ClientConfig& config);
    ~Client() = default;

    Client(Client&& client) noexcept;
    Client& operator=(Client&& client) noexcept;

    ///
    /// \~Chinese
    /// @brief 检查集群的连接状态。
    /// @details 发送Ping请求以测试指定数据库的连接是否正常。
    /// @param index 数据库集群索引。
    /// @param token
    /// 任务完成令牌，将在任务完成后被调用。若传递回调函数作为完成令牌，则其签名必须满足：
    /// @code void(std::exception_ptr) @endcode 若没有指定该参数，则使用默认值
    /// @ref token::Sync 。
    ///
    /// \~English
    /// @brief Check the status of cluster connection.
    /// @details Check conectivity with specified database through sending ping
    /// request.
    /// @param index Index of database.
    /// @param token The completion token which will be invoked when the task
    /// complete. If passing callback as token, The function signature must be
    /// @code void(std::exception_ptr) @endcode Default to @ref token::Sync if
    /// this parameter not specified.
    ///
    template<typename COMPLETION_TOKEN = token::Sync>
    [[nodiscard]] auto Ping(std::size_t index, COMPLETION_TOKEN&& token = {});

private:
    Client(const Client&)            = delete;
    Client& operator=(const Client&) = delete;

private:
    std::unique_ptr<impl::ClientImpl> impl_;
};

} // namespace opengemini

#include "opengemini/impl/Client.ipp"

#endif // !OPENGEMINI_CLIENT_HPP
