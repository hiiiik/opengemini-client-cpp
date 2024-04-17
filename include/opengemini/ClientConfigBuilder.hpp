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

#ifndef OPENGEMINI_CLIENTCONFIGBUILDER_HPP
#define OPENGEMINI_CLIENTCONFIGBUILDER_HPP

#include "opengemini/ClientConfig.hpp"

namespace opengemini {

///
/// \~Chinese
/// @brief 用于构建客户端配置的类。
/// @details 客户端配置各字段的具体含义 @ref ClientConfig 。
/// @note 必须在最后调用成员函数
/// @ref Finalize() 以获取最终的配置对象。
///
/// \~English
/// @brief Class to build client configuration.
/// @details For details about fileds of configuration see @ref ClientConfig.
/// @note The member function @ref Finalize() must be called at the end to get
/// the configuration object.
///
class ClientConfigBuilder {
private:
    using Self = ClientConfigBuilder;

public:
    ///
    /// \~Chinese
    /// @brief 默认构造函数。
    ///
    /// \~English
    /// @brief Default constructor.
    ///
    ClientConfigBuilder()  = default;
    ~ClientConfigBuilder() = default;

    ///
    /// \~Chinese
    /// @brief 结束构造并返回相应的配置。
    /// @return 客户端配置结构体。
    ///
    /// \~English
    /// @brief Finish bulding the client configuration and return it.
    /// @return The config struct as built.
    ///
    ClientConfig&& Finalize() noexcept;

    ///
    /// \~Chinese
    /// @brief 增加一个服务器地址以供客户端连接。
    /// @param address 服务器端点。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Append a server address to which the client will connect.
    /// @param address An endpoint to server.
    /// @return Reference to the builder itself.
    ///
    Self& AppendAddress(const Endpoint& address);

    ///
    /// \~Chinese
    /// @brief 增加多个服务器地址。
    /// @param addresses 服务器地址列表。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Append multiple server addresses to which the client will
    /// connect.
    /// @param addresses A vector of endpoints.
    /// @return Reference to the builder itself.
    ///
    Self& AppendAddresses(const std::vector<Endpoint>& addresses);

    ///
    /// \~Chinese
    /// @brief 设置鉴权时使用的用户凭据。
    /// @param username 用户名。
    /// @param password 密码。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the credential used for authentication.
    /// @param username
    /// @param password
    /// @return Reference to the builder itself.
    ///
    Self& AuthCredential(const std::string& username,
                         const std::string& password);

    ///
    /// \~Chinese
    /// @brief 设置鉴权时使用的用户令牌。
    /// @param token 用户令牌。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the token used for authentication.
    /// @param token
    /// @return Reference to the builder itself.
    ///
    Self& AuthToken(const std::string& token);

    ///
    /// \~Chinese
    /// @brief 设置点位写入功能的批量策略。
    /// @param interval 触发批量请求的时间间隔。
    /// @param size 触发批量请求的最大点位数量。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the strategy for write-point batching.
    /// @param interval Time interval that triggers a gather request.
    /// @param size Max number of points that triggers a gather request.
    /// @return Reference to the builder itself.
    ///
    Self& BatchConfig(std::chrono::seconds interval, std::size_t size);

    ///
    /// \~Chinese
    /// @brief 设置客户端读写超时。
    /// @param timeout 超时值。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the read/write timeout.
    /// @param timeout
    /// @return Reference to the builder itself.
    ///
    Self& ReadWriteTimeout(std::chrono::seconds timeout);

    ///
    /// \~Chinese
    /// @brief 设置客户端连接超时。
    /// @param 超时值。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the connect timeout.
    /// @param timeout
    /// @return Reference to the builder itself.
    ///
    Self& ConnectTimeout(std::chrono::seconds timeout);

    ///
    /// \~Chinese
    /// @brief 设置是否开启gzip。
    /// @param enabled
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set whether to enable gzip or not.
    /// @param enabled
    /// @return Reference to the builder itself.
    ///
    Self& EnableGzip(bool enabled);

    ///
    /// \~Chinese
    /// @brief 设置客户端并发参考值。
    /// @param hint 期望客户端运行的线程数。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the hint about the level of concurrency.
    /// @param hint A suggestion to the client on how many threads it should
    /// run.
    /// @return Reference to the builder itself.
    ///
    Self& ConcurrencyHint(std::size_t hint);

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
    ///
    /// \~Chinese
    /// @brief 设置是否跳过校验服务端证书。
    /// @param skipped
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set whehter to skip verifying the server's certificates or not.
    /// @param skippped
    /// @return Reference to the builder itself.
    ///
    Self& SkipVerifyPeer(bool skipped);

    ///
    /// \~Chinese
    /// @brief 设置客户端证书。
    /// @param certificates PEM格式的证书链。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set a certificate chain as the client's certificates.
    /// @param certificates A PEM format certificate chain.
    /// @return Reference to the builder itself.
    ///
    Self& ClientCertificates(const std::string& certificates);

    ///
    /// \~Chinese
    /// @brief 设置校验服务端证书时使用的根CA。
    /// @param rootCAs PEM格式的证书链。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set the trusted certificate authority for performing verfication.
    /// @param rootCAs A certificate chain in PEM format.
    /// @return Reference to the builder itself.
    ///
    Self& RootCAs(const std::string& rootCAs);

    ///
    /// \~Chinese
    /// @brief 设置SSL/TLS版本。
    /// @param version 版本。
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set version of SSL/TLS protocol.
    /// @param version
    /// @return Reference to the builder itself.
    ///
    Self& TLSVersion(TLSVersion version);

    ///
    /// \~Chinese
    /// @brief 设置是否开启SSL/TLS。
    /// @param enabled
    /// @return 指向配置构造器自身的引用。
    ///
    /// \~English
    /// @brief Set whether to enable SSL/TLS or not.
    /// @param enabled
    /// @return Reference to the builder itself.
    ///
    Self& EnableTLS(bool enabled);
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT

private:
    ClientConfigBuilder(const ClientConfigBuilder& builder)     = delete;
    ClientConfigBuilder(ClientConfigBuilder&& builder) noexcept = delete;

    ClientConfigBuilder& operator=(const ClientConfigBuilder& builder) = delete;
    ClientConfigBuilder&
    operator=(ClientConfigBuilder&& builder) noexcept = delete;

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
    TLSConfig& PrepareTLSConfig();
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT

    ClientConfig conf_;
};

} // namespace opengemini

#ifndef OPENGEMINI_SEPERATE_COMPILATION
#    include "opengemini/impl/ClientConfigBuilder.cpp"
#endif // !OPENGEMINI_SEPERATE_COMPILATION

#endif // !OPENGEMINI_CLIENTCONFIGBUILDER_HPP
