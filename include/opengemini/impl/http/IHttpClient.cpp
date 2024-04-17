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

#include "opengemini/impl/http/IHttpClient.hpp"

#include <algorithm>

#include <fmt/format.h>
#include <boost/asio/use_future.hpp>

#include "opengemini/Exceptions.hpp"
#include "opengemini/Version.hpp"
#include "opengemini/impl/util/Preprocessor.hpp"

namespace opengemini::impl::http {

OPENGEMINI_INLINE_SPECIFIER
IHttpClient::IHttpClient(std::chrono::seconds connectTimeout,
                         std::chrono::seconds readWriteTimeout,
                         std::size_t          threadNum) :
    ctx_(threadNum),
    connectTimeout_(connectTimeout),
    readWriteTimeout_(readWriteTimeout),
    ctxGuard_(boost::asio::make_work_guard(ctx_)),
    threads_(ConstructWorkingThreads(threadNum)),
    userAgent_([] {
        return fmt::format("opengemini-client-cxx/{}.{}.{}",
                           OPENGEMINI_VERSION_MAJOR,
                           OPENGEMINI_VERSION_MINOR,
                           OPENGEMINI_VERSION_PATCH);
    }())
{ }

OPENGEMINI_INLINE_SPECIFIER
IHttpClient::~IHttpClient()
{
    ctxGuard_.reset();
    ctx_.stop();
    for (auto& thread : threads_) {
        if (thread.joinable()) { thread.join(); }
    }
}

OPENGEMINI_INLINE_SPECIFIER
std::unordered_map<std::string, std::string>&
IHttpClient::DefaultHeaders() noexcept
{
    return headers_;
}

OPENGEMINI_INLINE_SPECIFIER
HttpRequest IHttpClient::BuildRequest(std::string              host,
                                      std::string              target,
                                      std::string              body,
                                      boost::beast::http::verb method) const
{
    HttpRequest request{ std::move(method),
                         std::move(target),
                         httpProtocolVersion_ };
    request.body() = std::move(body);
    for (const auto& header : headers_) {
        request.set(header.first, header.second);
    }
    request.set(boost::beast::http::field::host, std::move(host));
    request.set(boost::beast::http::field::user_agent, userAgent_);
    request.prepare_payload();

    return request;
}

OPENGEMINI_INLINE_SPECIFIER
std::vector<std::thread>
IHttpClient::ConstructWorkingThreads(std::size_t threadNum)
{
    if (threadNum <= 0) {
        throw InvalidArgument("thread num should be greater than zero");
    }

    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    std::generate_n(std::back_inserter(threads), threadNum, [this] {
        return std::thread([this] {
            for (;;) {
                try {
                    ctx_.run();
                    break;
                }
                catch (...) {
                }
            }
        });
    });
    return threads;
}

} // namespace opengemini::impl::http
