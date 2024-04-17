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

/// @file ClientConfig.hpp
#ifndef OPENGEMINI_CLIENTCONFIG_HPP
#define OPENGEMINI_CLIENTCONFIG_HPP

#include <chrono>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
#    include <boost/asio/ssl.hpp>
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT

#include "opengemini/Endpoint.hpp"

namespace opengemini {

///
/// \~Chinese
/// @brief 存放鉴权使用的用户凭据。
///
/// \~English
/// @brief Holds the user credentail for authorization.
///
struct AuthCredential {
    std::string username;
    std::string password;
};

///
/// \~Chinese
/// @brief 鉴权令牌。
///
/// \~English
/// @brief Authentication token.
///
using AuthToken = std::string;

///
/// \~Chinese
/// @brief 鉴权配置，只能是用户凭据或者用户令牌中的一种。
///
/// \~English
/// @brief Authentication configuration, which can only be either user
/// credential or token.
///
using AuthConfig = std::variant<AuthCredential, AuthToken>;

///
/// \~Chinese
/// @brief 批量配置，控制写入点位时的批量策略。
///
/// \~English
/// @brief Hold the configs that control the bulk strategy while writing points.
///
struct BatchConfig {
    ///
    /// \~Chinese
    /// @brief 触发批量请求的时间间隔。
    /// @details 调用 @ref WritePoint()
    /// 时，客户端可能不会立刻发送请求（而是等待更多调用以聚合为一次请求）。
    /// 如果时间间隔达到预设值，则立即发送一次聚合请求。
    ///
    /// \~English
    /// @brief Time interval that triggers a gather request.
    /// @details When calling @ref WritePoint() , the client may not send the
    /// request immediately (waiting for more calls to gather into one request).
    /// If the time interval reached, a gather request will be sent immediately.
    ///
    std::chrono::seconds batchInterval;

    ///
    /// \~Chinese
    /// @brief 触发批量请求的最大点位数量。
    /// @details 调用 @ref WritePoint()
    /// 时，客户端可能不会立刻发送请求（而是等待更多调用以聚合为一次请求）。
    /// 如果累计的点位数量超出最大值，则立即发送一次聚合请求。
    ///
    /// \~English
    /// @brief Max number of points that triggers a gather request.
    /// @details When calling @ref WritePoint() , the client may not send the
    /// request immediately (waiting for more calls to gather into one request).
    /// If the number of points exceeds the maximum size, a gather request will
    /// be sent immediately.
    ///
    std::size_t batchSize;
};

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT

///
/// \~Chinese
/// @brief SSL/TLS协议版本。
///
/// \~English
/// @brief The version of SSL/TLS protocol.
///
enum class TLSVersion {
    sslv2  = boost::asio::ssl::context::sslv2_client,
    sslv3  = boost::asio::ssl::context::sslv3_client,
    tlsv1  = boost::asio::ssl::context::tlsv1_client,
    tlsv11 = boost::asio::ssl::context::tlsv11_client,
    tlsv12 = boost::asio::ssl::context::tlsv12_client,
    tlsv13 = boost::asio::ssl::context::tlsv13_client,
};

///
/// \~Chinese
/// @brief 客户端TLS配置。
///
/// \~English
/// @brief The TLS configuration for client.
///
struct TLSConfig {
    ///
    /// \~Chinese
    /// @brief 是否跳过验证服务端证书，默认值为false。
    ///
    /// \~English
    /// @brief Whether to skip verifying the server's certificates, default to
    /// false.
    ///
    bool skipVerifyPeer{ false };

    ///
    /// \~Chinese
    /// @brief PEM格式的客户端证书链，默认值为空（不使用客户端证书）。
    ///
    /// \~English
    /// @brief A PEM format certificate chain as the client's certificates,
    /// default to an empty string(no client's certificates used).
    ///
    std::string certificates;

    ///
    /// \~Chinese
    /// @brief 校验服务端证书时使用的根CA。
    /// @details
    /// 必须是PEM格式的证书链。若没有指定该选项，则客户端将使用系统默认路径上存在的根CA。
    ///
    /// \~English
    /// @brief The certificate authority for performing verfication.
    /// @details Must be a certificate chain in PEM format. If not specified,
    /// the client will use CAs under the default system-dependent directories.
    ///
    std::string rootCAs;

    ///
    /// \~Chinese
    /// @brief 客户端使用的SSL/TLS版本，默认使用TLS v1.2。
    ///
    /// \~English
    /// @brief SSL/TLS version to use on the client, default to TLS v1.2.
    ///
    TLSVersion version{ TLSVersion::tlsv12 };
};

#endif // OPENGEMINI_ENABLE_SSL_SUPPORT

///
/// \~Chinese
/// @brief openGemini客户端配置。
///
/// \~English
/// @brief The configuration of openGemini client.
///
struct ClientConfig {
    ///
    /// \~Chinese
    /// @brief openGemini服务端地址列表。
    /// @details 可由一个或多个端点 @ref Endpoint
    /// 组成，不允许为空，也不允许包含重复端点。
    ///
    /// \~English
    /// @brief Addresses of openGemini server.
    /// @details Consist of one or more endpoints @ref Endpoint , and is not
    /// allowed to be empty or to contain duplicate endpoints.
    ///
    std::vector<Endpoint> addresses;

    ///
    /// \~Chinese
    /// @brief 客户端鉴权配置，默认值为 @code std::nullopt
    /// @endcode（无需鉴权）。
    ///
    /// \~English
    /// @brief Client authentication, default to @code std::nullopt @endcode (no
    /// authorization required).
    ///
    std::optional<AuthConfig> authConfig{ std::nullopt };

    ///
    /// \~Chinese
    /// @brief 聚合请求配置，默认值为 @code std::nullopt @endcode
    /// （不开启聚合请求）。
    ///
    /// \~English
    /// @brief Gather request configuration, default to @code std::nullopt
    /// @endcode (disable gather request).
    ///
    std::optional<BatchConfig> batchConfig{ std::nullopt };

    ///
    /// \~Chinese
    /// @brief 客户端读写超时，默认值为30秒。
    ///
    /// \~English
    /// @brief Client read/write timeout, default to 30 seconds.
    ///
    std::chrono::seconds timeout{ 30 };

    ///
    /// \~Chinese
    /// @brief 客户端连接超时，默认值为30秒。
    ///
    /// \~English
    /// @brief Client connect timeout, default to 30 seconds.
    ///
    std::chrono::seconds connectTimeout{ 30 };

    ///
    /// \~Chinese
    /// @brief 是否开启gzip，默认值为false。
    ///
    /// \~English
    /// @brief Whether to enable gzip, default to false.
    ///
    bool gzipEnabled{ false };

    ///
    /// \~Chinese
    /// @brief 客户端并发参考值。
    /// @details 期望客户端运行的线程数，客户端可能参考该值选择合适的线程数。
    ///
    /// \~English
    /// @brief A hint about the level of concurrency.
    /// @details A suggestion to the client on how many threads it should run.
    ///
    std::size_t concurrencyHint{ 0 };
#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT

    ///
    /// \~Chinese
    /// @brief 客户端TLS配置，默认值为@code std::nullopt @endcode
    /// (客户端内部将使用 @ref TLSConfig 默认值)。
    ///
    /// \~English
    /// @brief The Client TLS configuration, default to @code std::nullopt
    /// @endcode (Client will use the default value of @ref TLSConfig ).
    ///
    std::optional<TLSConfig> tlsConfig{ std::nullopt };

    ///
    /// \~Chinese
    /// @brief 是否开启TLS，默认值为false。
    ///
    /// \~English
    /// @brief Whether to enable TLS, default to false.
    ///
    bool tlsEnabled{ false };

#endif // OPENGEMINI_ENABLE_SSL_SUPPORT
};

} // namespace opengemini

#endif // !OPENGEMINI_CLIENTCONFIG_HPP
