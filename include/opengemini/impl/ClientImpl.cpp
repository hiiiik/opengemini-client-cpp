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

#include "opengemini/impl/ClientImpl.hpp"

#include <fmt/format.h>

#include "opengemini/Exceptions.hpp"
#include "opengemini/impl/http/HttpClient.hpp"
#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
#    include "opengemini/impl/http/HttpsClient.hpp"
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT
#include "opengemini/impl/util/Base64.hpp"
#include "opengemini/impl/util/Preprocessor.hpp"

namespace opengemini::impl {

OPENGEMINI_INLINE_SPECIFIER
ClientImpl::ClientImpl(const ClientConfig& config) :
    http_(ConstructHttpClient(config)),
    lb_(config.addresses, http_)
{ }

OPENGEMINI_INLINE_SPECIFIER
std::shared_ptr<http::IHttpClient>
ClientImpl::ConstructHttpClient(const ClientConfig& config) const
{
    auto buildHttp = [&config]() -> decltype(http_) {
        auto totalThreadNum = ChooseThreadsNum(config.concurrencyHint);
        // 3 threads for ClientImpl itself, lb_ and batch_
        auto httpThreadNum = totalThreadNum > 3 ? totalThreadNum - 3 : 1;

#ifdef OPENGEMINI_ENABLE_SSL_SUPPORT
        if (config.tlsEnabled) {
            return std::make_shared<http::HttpsClient>(
                config.tlsConfig.value_or(TLSConfig{}),
                config.connectTimeout,
                config.timeout,
                httpThreadNum);
        }
#endif // OPENGEMINI_ENABLE_SSL_SUPPORT

        return std::make_shared<http::HttpClient>(config.connectTimeout,
                                                  config.timeout,
                                                  httpThreadNum);
    };

    auto http = buildHttp();
    if (auto& auth = config.authConfig; auth.has_value()) {
        if (auto val = std::get_if<AuthCredential>(&auth.value())) {
            auto cred = fmt::format("{}:{}", val->username, val->password);
            http->DefaultHeaders()["Authorization"] =
                fmt::format("Basic {}", util::Base64Encode(cred));
        }
        else {
            throw NotImplemented("only support authorization credential");
        }
    }

    return http;
}

OPENGEMINI_INLINE_SPECIFIER
std::size_t ClientImpl::ChooseThreadsNum(std::size_t concurrencyHint) noexcept
{
    constexpr auto min          = 1;
    const auto     hardwareHint = std::thread::hardware_concurrency();
    const auto     max          = hardwareHint * 4;
    if (concurrencyHint >= min && concurrencyHint <= max) {
        return concurrencyHint;
    }

    return hardwareHint == 0 ? min : hardwareHint;
}

} // namespace opengemini::impl
