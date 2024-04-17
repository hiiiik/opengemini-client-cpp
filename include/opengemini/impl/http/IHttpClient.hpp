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

#ifndef OPENGEMINI_IMPL_HTTP_IHTTPCLIENT_HPP
#define OPENGEMINI_IMPL_HTTP_IHTTPCLIENT_HPP

#include <chrono>
#include <exception>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>

#include "opengemini/Endpoint.hpp"

namespace opengemini::impl::http {

using HttpStatus = boost::beast::http::status;
using HttpRequest =
    boost::beast::http::request<boost::beast::http::string_body>;
using HttpResponse =
    boost::beast::http::response<boost::beast::http::string_body>;

class IHttpClient {
public:
    using CompletionSignature = void(std::exception_ptr, HttpResponse);

public:
    IHttpClient(std::chrono::seconds connectTimeout,
                std::chrono::seconds readWriteTimeout,
                std::size_t          threadNum);

    virtual ~IHttpClient();

    template<typename COMPLETION_TOKEN>
    auto
    Get(const Endpoint& endpoint, std::string target, COMPLETION_TOKEN&& token);

    template<typename COMPLETION_TOKEN>
    auto Post(const Endpoint&    endpoint,
              std::string        target,
              std::string        body,
              COMPLETION_TOKEN&& token);

    std::unordered_map<std::string, std::string>& DefaultHeaders() noexcept;

protected:
    virtual HttpResponse SendRequest(const Endpoint&            endpoint,
                                     HttpRequest                request,
                                     boost::asio::yield_context yield) = 0;

protected:
    boost::asio::io_context                      ctx_;
    std::unordered_map<std::string, std::string> headers_;

    const std::chrono::seconds connectTimeout_;
    const std::chrono::seconds readWriteTimeout_;

private:
    IHttpClient(const IHttpClient&)            = delete;
    IHttpClient(IHttpClient&&)                 = delete;
    IHttpClient& operator=(const IHttpClient&) = delete;
    IHttpClient& operator=(IHttpClient&&)      = delete;

    template<typename COMPLETION_TOKEN>
    auto SpawnRequest(const Endpoint&    endpoint,
                      HttpRequest&&      request,
                      COMPLETION_TOKEN&& handler);

    HttpRequest BuildRequest(std::string              host,
                             std::string              target,
                             std::string              body,
                             boost::beast::http::verb method) const;

    std::vector<std::thread> ConstructWorkingThreads(std::size_t threadNum);

private:
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
                             ctxGuard_;
    std::vector<std::thread> threads_;

    const std::string     userAgent_;
    static constexpr auto httpProtocolVersion_{ 11 };
};

} // namespace opengemini::impl::http

#include "opengemini/impl/http/IHttpClient.tpp"
#ifndef OPENGEMINI_SEPERATE_COMPILATION
#    include "opengemini/impl/http/IHttpClient.cpp"
#endif // !OPENGEMINI_SEPERATE_COMPILATION

#endif // !OPENGEMINI_IMPL_HTTP_IHTTPCLIENT_HPP
