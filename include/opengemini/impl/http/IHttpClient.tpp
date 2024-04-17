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

namespace opengemini::impl::http {

template<typename COMPLETION_TOKEN>
auto IHttpClient::SpawnRequest(const Endpoint&    endpoint,
                               HttpRequest&&      request,
                               COMPLETION_TOKEN&& handler)
{
    return boost::asio::spawn(
        ctx_,
        [this, endpoint, request = std::move(request)](auto yield) mutable {
            return SendRequest(std::move(endpoint), std::move(request), yield);
        },
        std::forward<COMPLETION_TOKEN>(handler));
}

template<typename COMPLETION_TOKEN>
auto IHttpClient::Get(const Endpoint&    endpoint,
                      std::string        target,
                      COMPLETION_TOKEN&& token)
{
    return SpawnRequest(endpoint,
                        BuildRequest(endpoint.host,
                                     std::move(target),
                                     {},
                                     boost::beast::http::verb::get),
                        std::forward<COMPLETION_TOKEN>(token));
}

template<typename COMPLETION_TOKEN>
auto IHttpClient::Post(const Endpoint&    endpoint,
                       std::string        target,
                       std::string        body,
                       COMPLETION_TOKEN&& token)
{
    return SpawnRequest(endpoint,
                        BuildRequest(endpoint.host,
                                     std::move(target),
                                     std::move(body),
                                     boost::beast::http::verb::post),
                        std::forward<COMPLETION_TOKEN>(token));
}

} // namespace opengemini::impl::http
